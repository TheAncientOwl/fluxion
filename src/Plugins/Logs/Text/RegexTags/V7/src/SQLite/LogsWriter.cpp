/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsWriter.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation of @see LogsWriter.hpp
///

#include "LogsWriter.hpp"
#include <string_view>
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::LogsWriter);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::LogsWriter);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite {

LogsWriter::LogsWriter(DatabaseRef db, std::vector<std::string> const& fields) : m_database{db}
{
    LOG_SCOPE("::LogsWriter()");

    auto const fields_size{fields.size()};

    // Generate fields SQL
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

    // Prepare statement
    std::string const logs_sql =
        "INSERT INTO logs (id, " + fields_sql + ") VALUES (?, " + fields_sql_placeholders + ");";
    m_logs_statement = m_database.Prepare(logs_sql);
}

bool LogsWriter::WriteChunk(
    std::vector<std::vector<std::string_view>> const& rows,
    std::size_t const active_rows,
    std::vector<Data::FilteredLog>& out_filtered_logs)
{
    LOG_SCOPE("::WriteChunk()");

    if (active_rows == 0)
    {
        return true;
    }

    if (!m_logs_statement.IsValid())
    {
        LOG_ERROR("::WriteChunk(): Invalid statement: {}", m_database.GetLastErrorMessage());
        return false;
    }

    auto const num_fields = rows[0].size();

    for (std::size_t row_idx = 0; row_idx < active_rows; ++row_idx)
    {
        auto const& row = rows[row_idx];
        m_logs_statement.BindInt64(1, ++m_log_id);

        for (std::size_t field_idx = 0; field_idx < num_fields; ++field_idx)
        {
            m_logs_statement.BindTextStatic(static_cast<int>(2 + field_idx), row[field_idx]);
        }

        if (m_logs_statement.Step() != EStepResult::Done)
        {
            LOG_ERROR(
                "::WriteChunk(): Failed to insert into logs: {}", m_database.GetLastErrorMessage());
            m_logs_statement.Reset();
            continue;
        }

        out_filtered_logs.emplace_back(m_log_id);

        m_logs_statement.Reset();
    }

    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite
