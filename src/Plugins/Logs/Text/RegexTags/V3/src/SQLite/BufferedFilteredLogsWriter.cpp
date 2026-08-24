/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedFilteredLogsWriter.cpp
/// @author Alexandru Delegeanu
/// @version 3.3
/// @brief Implementation of @see BufferedFilteredLogsWriter.hpp
///

#include <stdexcept>

#include "BufferedFilteredLogsWriter.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedFilteredLogsWriter);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::BufferedFilteredLogsWriter);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

BufferedFilteredLogsWriter::BufferedFilteredLogsWriter(DatabaseRef db, std::size_t const batch_size)
    : m_database{db}, m_batch_size{batch_size}
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
    LOG_SCOPE("::ClearTable():");

    std::string err_msg{};
    if (!m_database.Execute("DELETE FROM filtered_logs", &err_msg))
    {
        LOG_ERROR(
            "::BufferedFilteredLogsWriter(): Failed to drop/create filtered_logs table: {}",
            err_msg.empty() ? "unknown error" : err_msg);
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

    char const* insert_sql =
        "INSERT INTO filtered_logs (view_index, log_id, filter_id, highlight_filter_id) VALUES (?, "
        "?, ?, ?);";

    Statement statement = m_database.Prepare(insert_sql);
    if (!statement.IsValid())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to prepare filtered_logs statement: {}",
            m_database.GetLastErrorMessage());
        return false;
    }

    for (std::size_t i = 0; i < m_current_index; ++i)
    {
        auto const& row = m_buffer[i];

        statement.BindInt64(1, ++m_view_index);
        statement.BindInt64(2, static_cast<std::int64_t>(row.log_id));
        statement.BindText(3, row.filter_id);
        statement.BindText(4, row.highlight_filter_id);

        if (statement.Step() != EStepResult::Done)
        {
            LOG_ERROR(
                "::ExecuteFlush(): Failed to insert into filtered_logs: {}",
                m_database.GetLastErrorMessage());
        }

        statement.Reset();
    }

    m_current_index = 0;
    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
