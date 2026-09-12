/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 6.5
/// @brief Implementation @see RegexTags.hpp
///

#include <filesystem>
#include <span>
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

void RegexTags::GetLogsABI(Bridge::ABI::Span<Bridge::Range> const _ranges, Bridge::ILogsWriter* out_logs)
{
    LOG_SCOPE("::GetLogsABI()");

    if (!out_logs)
    {
        return;
    }

    if (m_filtered_logs.empty())
    {
        return;
    }

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogsABI(): No logs were imported before");
        return;
    }

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::GetLogsABI(): SQLite connection is closed and could not be opened");
        return;
    }

    std::span<Bridge::Range const> const ranges{_ranges};

    std::stringstream ss{};
    for (auto const& range : ranges)
    {
        ss << "[" << range.begin << ", " << range.end << "), ";
    }
    LOG_INFO("::GetLogsABI(): Requested ranges: {}", ss.str());

    std::error_code ec;
    auto const db_path = MakeDatabasePath(*m_last_imported_logs_path);
    if (std::filesystem::file_size(db_path, ec) == 0 || ec)
    {
        LOG_WARN(
            "::GetLogsABI(): Database file {} is currently 0 bytes or locked. Skipping read.", db_path);
        return;
    }

    if (m_imported_logs_header.empty())
    {
        LOG_WARN("::GetLogsABI(): m_imported_logs_header is empty.");
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

            out_logs->WriteMetadata(
                view_idx, filtered_item.filter_id, filtered_item.highlight_filter_id);

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
        LOG_ERROR("::GetLogsABI(): Failed to prepare logs query.");
        return;
    }

    while (query_handle.Step() == SQLite::EStepResult::Row)
    {
        auto const log_id = static_cast<std::uint64_t>(query_handle.GetColumnInt64(0));
        auto const col_count = static_cast<std::size_t>(query_handle.GetColumnCount());

        std::vector<Bridge::ABI::StringView> columns;
        columns.reserve(col_count > 0 ? col_count - 1 : 0);
        for (std::size_t i = 1; i < col_count; ++i)
        {
            const char* text = query_handle.GetColumnText(static_cast<int>(i));
            columns.emplace_back(text ? text : "");
        }

        if (auto it = log_id_to_view_indices.find(log_id); it != log_id_to_view_indices.end())
        {
            for (std::size_t view_index : it->second)
            {
                out_logs->WriteData(view_index, columns);
            }
        }
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6
