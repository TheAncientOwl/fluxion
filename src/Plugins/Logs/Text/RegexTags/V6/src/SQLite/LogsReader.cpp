/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsReader.cpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Implementation of @see LogsReader.hpp
///

#include "LogsReader.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::LogsReader);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::LogsReader);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite {

LogsReader::LogsReader(DatabaseRef db) : m_database{db}
{
    LOG_SCOPE("::LogsReader()");
}

Statement LogsReader::PrepareGetAllLogsQuery(std::vector<std::string> const& fields)
{
    LOG_SCOPE("::PrepareGetAllLogsQuery()");

    if (fields.empty())
    {
        LOG_WARN("::PrepareGetAllLogsQuery(): Fields vector is empty.");
        return Statement{};
    }

    std::string fields_sql_str{"id"};
    for (auto const& field : fields)
    {
        fields_sql_str += ", " + field;
    }

    std::string const query_str = "SELECT " + fields_sql_str + " FROM logs ORDER BY id ASC;";
    LOG_INFO("::PrepareGetAllLogsQuery(): query == {}", query_str);

    Statement statement = m_database.Prepare(query_str);
    if (!statement.IsValid())
    {
        LOG_ERROR(
            "::PrepareGetAllLogsQuery(): Failed to prepare statement: {}",
            m_database.GetLastErrorMessage());
        return Statement{};
    }

    return statement;
}

bool LogsReader::NextRow(Statement& statement, std::size_t& out_log_id, std::vector<std::string>& out_fields)
{
    if (!statement.IsValid())
    {
        return false;
    }

    EStepResult const result = statement.Step();

    if (result == EStepResult::Row)
    {
        auto const col_count = statement.GetColumnCount();
        if (col_count < 2)
        {
            LOG_ERROR("::NextRow(): Column count is unexpectedly less than 2 (total: {}).", col_count);
            return false;
        }

        // Column 0 is logs.id
        out_log_id = static_cast<std::size_t>(statement.GetColumnInt64(0));

        // Remaining columns are fields (1 .. col_count - 1)
        std::size_t const num_fields = col_count - 1;
        out_fields.resize(num_fields);

        for (std::size_t idx = 0; idx < num_fields; ++idx)
        {
            char const* text = statement.GetColumnText(static_cast<int>(idx + 1));
            out_fields[idx] = text ? text : "";
        }

        return true;
    }
    else if (result == EStepResult::Done)
    {
        return false;
    }
    else
    {
        LOG_ERROR("::NextRow(): Execution error: {}", m_database.GetLastErrorMessage());
        return false;
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite
