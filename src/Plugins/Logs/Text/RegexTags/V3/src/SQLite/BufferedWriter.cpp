/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedWriter.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation of @see BufferedWriter.hpp
///

#include "BufferedWriter.hpp"

#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedWriter);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedWriter);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

BufferedWriter::BufferedWriter(
    std::filesystem::path db_path,
    std::size_t const batch_size,
    std::vector<std::string> const& fields)
    : OpenCloseManager{std::move(db_path)}, m_batch_size{batch_size}
{
    LOG_SCOPE("::BufferedWriter()");

    m_buffer.resize(m_batch_size);
    auto const fields_size{fields.size()};
    for (auto& row : m_buffer)
    {
        row.resize(fields_size);
    }

    for (std::size_t i = 0; i < fields_size; ++i)
    {
        m_fields += fields[i];
        m_fields_sql_placeholders += "?";
        if (i + 1 < fields_size)
        {
            m_fields += ", ";
            m_fields_sql_placeholders += ", ";
        }
    }
}

BufferedWriter::~BufferedWriter()
{
    Flush();
}

std::vector<std::string>& BufferedWriter::NextFrame()
{
    if (m_current_index >= m_batch_size)
    {
        if (!Flush())
        {
            throw std::runtime_error(
                "BufferedWriter::NextFrame(): Failed to flush buffer to database.");
        }
    }

    return m_buffer[m_current_index++];
}

bool BufferedWriter::Flush()
{
    if (m_current_index == 0)
    {
        return true;
    }
    return ExecuteFlush();
}

bool BufferedWriter::ExecuteFlush()
{
    LOG_SCOPE("::ExecuteFlush()");
    if (!m_db)
    {
        LOG_ERROR("::ExecuteFlush(): Database is not open!");
        return false;
    }

    char* err_msg = nullptr;
    if (sqlite3_exec(m_db.get(), "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        LOG_ERROR("::ExecuteFlush(): Failed to begin transaction: {}", err_msg ? err_msg : "unknown");
        if (err_msg)
        {
            sqlite3_free(err_msg);
        }
        return false;
    }

    auto const num_fields = m_buffer[0].size();

    // 1. Prepare statement for the logs table
    std::string logs_sql =
        "INSERT INTO logs (" + m_fields + ") VALUES (" + m_fields_sql_placeholders + ");";
    sqlite3_stmt* stmt_logs = nullptr;
    if (sqlite3_prepare_v2(m_db.get(), logs_sql.c_str(), -1, &stmt_logs, nullptr) != SQLITE_OK)
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare logs statement: {}", sqlite3_errmsg(m_db.get()));
        sqlite3_exec(m_db.get(), "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    // 2. Prepare statement for the filtered_logs table
    // Initially inserting NULL for filter IDs during raw import (will be populated later during filtering passes)
    const char* filtered_sql =
        "INSERT INTO filtered_logs (log_id, filter_id, highlight_filter_id) VALUES (?, NULL, "
        "NULL);";
    sqlite3_stmt* stmt_filtered = nullptr;
    if (sqlite3_prepare_v2(m_db.get(), filtered_sql, -1, &stmt_filtered, nullptr) != SQLITE_OK)
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare filtered_logs statement: {}",
            sqlite3_errmsg(m_db.get()));
        sqlite3_finalize(stmt_logs);
        sqlite3_exec(m_db.get(), "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    // Only iterate up to the exact frames we used
    for (std::size_t i = 0; i < m_current_index; ++i)
    {
        auto const& row = m_buffer[i];

        // Bind dynamic log fields
        for (std::size_t f = 0; f < num_fields; ++f)
        {
            sqlite3_bind_text(stmt_logs, static_cast<int>(1 + f), row[f].c_str(), -1, SQLITE_TRANSIENT);
        }

        if (sqlite3_step(stmt_logs) != SQLITE_DONE)
        {
            LOG_ERROR("::ExecuteFlush(): Failed to insert into logs: {}", sqlite3_errmsg(m_db.get()));
            sqlite3_reset(stmt_logs);
            continue;
        }

        // Retrieve the auto-generated primary key ID of the newly inserted log record
        sqlite3_int64 const log_id = sqlite3_last_insert_rowid(m_db.get());

        // Bind log_id to filtered_logs (filter_id and highlight_filter_id default to NULL on raw insert)
        sqlite3_bind_int64(stmt_filtered, 1, log_id);

        if (sqlite3_step(stmt_filtered) != SQLITE_DONE)
        {
            LOG_ERROR(
                "::ExecuteFlush(): Failed to insert into filtered_logs: {}",
                sqlite3_errmsg(m_db.get()));
        }

        sqlite3_reset(stmt_logs);
        sqlite3_reset(stmt_filtered);
    }

    sqlite3_finalize(stmt_logs);
    sqlite3_finalize(stmt_filtered);

    if (sqlite3_exec(m_db.get(), "COMMIT;", nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        LOG_ERROR("::ExecuteFlush(): Failed to commit transaction: {}", err_msg ? err_msg : "unknown");
        if (err_msg)
        {
            sqlite3_free(err_msg);
        }
        sqlite3_exec(m_db.get(), "ROLLBACK;", nullptr, nullptr, nullptr);
        m_current_index = 0;
        return false;
    }

    // Reset index to reuse the memory pool without reallocating
    m_current_index = 0;
    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
