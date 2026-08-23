/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedFilteredLogsWriter.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation of @see BufferedFilteredLogsWriter.hpp
///

#include "BufferedFilteredLogsWriter.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedFilteredLogsWriter);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedFilteredLogsWriter);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

BufferedFilteredLogsWriter::BufferedFilteredLogsWriter(std::filesystem::path db_path, std::size_t const batch_size)
    : OpenCloseManager{std::move(db_path)}, m_batch_size{batch_size}
{
    LOG_SCOPE("::BufferedFilteredLogsWriter()");
    m_buffer.resize(m_batch_size);
}

BufferedFilteredLogsWriter::~BufferedFilteredLogsWriter()
{
    Flush();
}

bool BufferedFilteredLogsWriter::ClearTable()
{
    char* err_msg = nullptr;
    const char* filtered_logs_table_sql =
        "DROP TABLE IF EXISTS filtered_logs;"
        "CREATE TABLE filtered_logs ("
        "    view_index INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    log_id INTEGER NOT NULL,"
        "    filter_id TEXT,"
        "    highlight_filter_id TEXT,"
        "    FOREIGN KEY(log_id) REFERENCES logs(id)"
        ");";

    if (sqlite3_exec(m_db.get(), filtered_logs_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        LOG_ERROR(
            "::BufferedFilteredLogsWriter(): Failed to drop/create filtered_logs table: {}",
            err_msg ? err_msg : "unknown error");
        if (err_msg)
        {
            sqlite3_free(err_msg);
        }
        return false;
    }
    return true;
}

BufferedFilteredLogsWriter::FilteredRow& BufferedFilteredLogsWriter::NextFrame()
{
    if (m_current_index >= m_batch_size)
    {
        if (!Flush())
        {
            throw std::runtime_error(
                "BufferedFilteredLogsWriter::NextFrame(): Failed to flush buffer to database.");
        }
    }

    return m_buffer[m_current_index++];
}

bool BufferedFilteredLogsWriter::Flush()
{
    if (m_current_index == 0)
    {
        return true;
    }
    return ExecuteFlush();
}

bool BufferedFilteredLogsWriter::ExecuteFlush()
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

    const char* insert_sql =
        "INSERT INTO filtered_logs (log_id, filter_id, highlight_filter_id) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db.get(), insert_sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare filtered_logs statement: {}",
            sqlite3_errmsg(m_db.get()));
        sqlite3_exec(m_db.get(), "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    for (std::size_t i = 0; i < m_current_index; ++i)
    {
        auto const& row = m_buffer[i];

        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(row.log_id));
        sqlite3_bind_text(stmt, 2, row.filter_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, row.highlight_filter_id.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            LOG_ERROR(
                "::ExecuteFlush(): Failed to insert into filtered_logs: {}",
                sqlite3_errmsg(m_db.get()));
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

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

    m_current_index = 0;
    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
