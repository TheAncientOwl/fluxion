/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedWriter.cpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Implementation of @see BufferedWriter.hpp
///

#include <stdexcept>

#include "BufferedWriter.hpp"
#include "Graphite/Logger.hpp"
#include "Wrapper/Transaction.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite::BufferedWriter);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite::BufferedWriter);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite {

BufferedWriter::BufferedWriter(
    DatabaseRef db,
    std::size_t const batch_size,
    std::vector<std::string> const& fields)
    : m_database{db}, m_batch_size{batch_size}
{
    LOG_SCOPE("::BufferedWriter()");

    // 1. Allocate buffer
    m_buffer.resize(m_batch_size);
    auto const fields_size{fields.size()};
    for (auto& row : m_buffer)
    {
        row.resize(fields_size);
    }

    // 2. Generate fields sql
    std::string fields_sql{""};
    std::string fields_sql_placeholders{""};
    for (std::size_t i = 0; i < fields_size; ++i)
    {
        fields_sql += fields[i];
        fields_sql_placeholders += "?";
        if (i + 1 < fields_size)
        {
            fields_sql += ", ";
            fields_sql_placeholders += ", ";
        }
    }

    // 3. Create logs statement
    std::string const logs_sql =
        "INSERT INTO logs (id, " + fields_sql + ") VALUES (?, " + fields_sql_placeholders + ");";
    m_logs_statement = m_database.Prepare(logs_sql);

    // 4. Create filtered logs statement
    char const* filtered_sql =
        "INSERT INTO filtered_logs (view_index, log_id, filter_id, highlight_filter_id) VALUES (?, "
        "?, NULL, NULL);";
    m_filtered_logs_statement = m_database.Prepare(filtered_sql);
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

    if (!m_logs_statement.IsValid())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare logs statement: {}",
            m_database.GetLastErrorMessage());
        return false;
    }

    if (!m_filtered_logs_statement.IsValid())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare filtered_logs statement: {}",
            m_database.GetLastErrorMessage());
        return false;
    }

    for (std::size_t i = 0; i < m_current_index; ++i)
    {
        auto const& row = m_buffer[i];

        m_logs_statement.BindInt64(1, ++m_log_id);
        // Bind dynamic log fields
        for (std::size_t fieldIndex = 0; fieldIndex < num_fields; ++fieldIndex)
        {
            m_logs_statement.BindText(static_cast<int>(2 + fieldIndex), row[fieldIndex]);
        }

        if (m_logs_statement.Step() != EStepResult::Done)
        {
            LOG_ERROR(
                "::ExecuteFlush(): Failed to insert into logs: {}", m_database.GetLastErrorMessage());
            m_logs_statement.Reset();
            continue;
        }

        // Bind log_id to filtered_logs
        m_filtered_logs_statement.BindInt64(1, m_log_id);
        m_filtered_logs_statement.BindInt64(2, m_log_id);

        if (m_filtered_logs_statement.Step() != EStepResult::Done)
        {
            LOG_ERROR(
                "::ExecuteFlush(): Failed to insert into filtered_logs: {}",
                m_database.GetLastErrorMessage());
        }

        m_logs_statement.Reset();
        m_filtered_logs_statement.Reset();
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

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite
