/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 6.1
/// @brief Implementation @see RegexTags.hpp
///

#include <filesystem>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V6/RegexTags.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"
#include "SQLite/Utility.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6 {

void RegexTags::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs)
{
    LOG_SCOPE("::GetLogs()");

    if (m_filtered_logs.empty())
    {
        return;
    }

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::GetLogs(): SQLite connection is closed and could not be opened");
        return;
    }

    std::stringstream ss{};
    for (auto const& range : ranges)
    {
        ss << "[" << range.begin << ", " << range.end << "), ";
    }
    LOG_INFO("::GetLogs(): Requested ranges: {}", ss.str());

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogs(): No logs were imported before");
        return;
    }

    std::error_code ec;
    auto const db_path = MakeDatabasePath(*m_last_imported_logs_path);
    if (std::filesystem::file_size(db_path, ec) == 0 || ec)
    {
        LOG_WARN(
            "::GetLogs(): Database file {} is currently 0 bytes or locked. Skipping read.", db_path);
        return;
    }

    if (m_imported_logs_header.empty())
    {
        LOG_WARN("::GetLogs(): m_imported_logs_header is empty.");
        return;
    }

    std::vector<std::uint64_t> log_ids_to_fetch;
    std::unordered_map<std::uint64_t, std::vector<std::size_t>> log_id_to_view_indices;

    for (auto const& range : ranges)
    {
        for (std::size_t view_idx = range.begin;
             view_idx < range.end && view_idx < m_filtered_logs.size();
             ++view_idx)
        {
            auto const& filtered_item = m_filtered_logs[view_idx];

            auto& target_row = out_logs[view_idx];
            target_row.metadata = {
                .filter_id = filtered_item.filter_id,
                .highlight_id = filtered_item.highlight_filter_id};

            log_id_to_view_indices[filtered_item.log_id].push_back(view_idx);
            log_ids_to_fetch.push_back(filtered_item.log_id);
        }
    }

    if (log_ids_to_fetch.empty())
    {
        return;
    }

    auto reader = SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()};
    auto query_handle = reader.PrepareGetLogsByIDsQuery(
        log_ids_to_fetch, SQLite::Utility::MakeFieldsIDs(m_imported_logs_header));

    if (!query_handle.IsValid())
    {
        LOG_ERROR("::GetLogs(): Failed to prepare logs query.");
        return;
    }

    while (query_handle.Step() == SQLite::EStepResult::Row)
    {
        auto const log_id = static_cast<std::uint64_t>(query_handle.GetColumnInt64(0));
        auto const col_count = static_cast<std::size_t>(query_handle.GetColumnCount());

        std::vector<std::string> fields(col_count - 1);
        for (std::size_t i = 1; i < col_count; ++i)
        {
            const char* text = query_handle.GetColumnText(static_cast<int>(i));
            fields[i - 1] = text ? text : "";
        }

        if (auto it = log_id_to_view_indices.find(log_id); it != log_id_to_view_indices.end())
        {
            for (std::size_t view_index : it->second)
            {
                out_logs[view_index].data = fields;
            }
        }
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6
