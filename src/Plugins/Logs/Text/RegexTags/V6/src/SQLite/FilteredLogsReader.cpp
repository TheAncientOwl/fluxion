/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FilteredLogsReader.cpp
/// @author Alexandru Delegeanu
/// @version 6.1
/// @brief Implementation of @see FilteredLogsReader.hpp
///

#include "FilteredLogsReader.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::FilteredLogsReader);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::FilteredLogsReader);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite {

FilteredLogsReader::FilteredLogsReader(DatabaseRef db) : m_database{db}
{
    LOG_SCOPE("::FilteredLogsReader()");
}

Statement FilteredLogsReader::PrepareGetLogsByIDsQuery(
    std::vector<std::uint64_t> const& log_ids,
    std::vector<std::string> const& fields)
{
    LOG_SCOPE("::PrepareGetLogsByIDsQuery()");

    if (log_ids.empty() || fields.empty())
    {
        return Statement{};
    }

    // Build fields list
    std::string fields_sql_str{};
    for (std::size_t i = 0; i < fields.size(); ++i)
    {
        if (i > 0)
            fields_sql_str += ", ";
        fields_sql_str += fields[i];
    }

    // Direct query against logs table without JOIN
    std::string query_str = "SELECT id, " + fields_sql_str + " FROM logs WHERE id IN (";
    for (std::size_t i = 0; i < log_ids.size(); ++i)
    {
        if (i > 0)
            query_str += ",";
        query_str += std::to_string(log_ids[i]);
    }
    query_str += ");";

    Statement statement = m_database.Prepare(query_str);
    if (!statement.IsValid())
    {
        LOG_ERROR(
            "::PrepareGetLogsByIDsQuery(): Failed to prepare statement: {}",
            m_database.GetLastErrorMessage());
        return Statement{};
    }

    return statement;
}

std::optional<std::size_t> FilteredLogsReader::GetNextFilteredIndex(
    std::vector<Data::FilteredLog> const& filtered_logs,
    Graphite::Common::Utility::UniqueID const& target_filter_id,
    std::size_t current_index)
{
    LOG_SCOPE("::GetNextFilteredIndex()");

    if (filtered_logs.empty())
        return std::nullopt;

    auto matches = [&](Data::FilteredLog const& item) {
        return item.filter_id == target_filter_id || item.highlight_filter_id == target_filter_id;
    };

    // Forward search from current_index + 1
    for (std::size_t i = current_index + 1; i < filtered_logs.size(); ++i)
    {
        if (matches(filtered_logs[i]))
            return i;
    }

    // Wrap around to start
    for (std::size_t i = 0; i <= current_index && i < filtered_logs.size(); ++i)
    {
        if (matches(filtered_logs[i]))
            return i;
    }

    return std::nullopt;
}

std::optional<std::size_t> FilteredLogsReader::GetPrevFilteredIndex(
    std::vector<Data::FilteredLog> const& filtered_logs,
    Graphite::Common::Utility::UniqueID const& target_filter_id,
    std::size_t current_index)
{
    LOG_SCOPE("::GetPrevFilteredIndex()");

    if (filtered_logs.empty())
        return std::nullopt;

    auto matches = [&](Data::FilteredLog const& item) {
        return item.filter_id == target_filter_id || item.highlight_filter_id == target_filter_id;
    };

    // Backward search from current_index - 1
    if (current_index > 0)
    {
        for (std::size_t i = current_index - 1;; --i)
        {
            if (matches(filtered_logs[i]))
                return i;
            if (i == 0)
                break;
        }
    }

    // Wrap around to end
    for (std::size_t i = filtered_logs.size() - 1; i >= current_index; --i)
    {
        if (matches(filtered_logs[i]))
            return i;
        if (i == 0)
            break;
    }

    return std::nullopt;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite
