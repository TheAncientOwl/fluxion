/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedWriter.cpp
/// @author Alexandru Delegeanu
/// @version 3.1
/// @brief Implementation of @see BufferedWriter.hpp
///

#include <stdexcept>

#include "BufferedWriter.hpp"
#include "Graphite/Logger.hpp"
#include "Wrapper/Transaction.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedWriter);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedWriter);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

BufferedWriter::BufferedWriter(
    DatabaseRef db,
    std::size_t const batch_size,
    std::vector<std::string> const& fields)
    : m_database{db}, m_batch_size{batch_size}
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

    Transaction transaction{m_database};
    if (!transaction.IsActive())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to begin transaction: {}", m_database.GetLastErrorMessage());
        return false;
    }

    auto const num_fields = m_buffer[0].size();

    // 1. Prepare statement for the logs table
    std::string const logs_sql =
        "INSERT INTO logs (" + m_fields + ") VALUES (" + m_fields_sql_placeholders + ");";

    Statement statement_logs = m_database.Prepare(logs_sql);
    if (!statement_logs.IsValid())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare logs statement: {}",
            m_database.GetLastErrorMessage());
        return false;
    }

    // 2. Prepare statement for the filtered_logs table
    char const* filtered_sql =
        "INSERT INTO filtered_logs (log_id, filter_id, highlight_filter_id) VALUES (?, NULL, "
        "NULL);";

    Statement statement_filtered = m_database.Prepare(filtered_sql);
    if (!statement_filtered.IsValid())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare filtered_logs statement: {}",
            m_database.GetLastErrorMessage());
        return false;
    }

    for (std::size_t i = 0; i < m_current_index; ++i)
    {
        auto const& row = m_buffer[i];

        // Bind dynamic log fields
        for (std::size_t f = 0; f < num_fields; ++f)
        {
            statement_logs.BindText(static_cast<int>(1 + f), row[f]);
        }

        if (statement_logs.Step() != EStepResult::Done)
        {
            LOG_ERROR(
                "::ExecuteFlush(): Failed to insert into logs: {}", m_database.GetLastErrorMessage());
            statement_logs.Reset();
            continue;
        }

        // Retrieve the auto-generated primary key ID
        std::int64_t const log_id = m_database.GetLastInsertRowId();

        // Bind log_id to filtered_logs
        statement_filtered.BindInt64(1, log_id);

        if (statement_filtered.Step() != EStepResult::Done)
        {
            LOG_ERROR(
                "::ExecuteFlush(): Failed to insert into filtered_logs: {}",
                m_database.GetLastErrorMessage());
        }

        statement_logs.Reset();
        statement_filtered.Reset();
    }

    if (!transaction.Commit())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to commit transaction: {}", m_database.GetLastErrorMessage());
        m_current_index = 0;
        return false;
    }

    m_current_index = 0;
    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
