/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 8.0
/// @brief Implementation @see RegexTags.hpp
///

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V8/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8::GetLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8::GetLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V8 {

void RegexTags::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs)
{
    LOG_SCOPE("::GetLogs()");

    if (m_filtered_logs.empty() || ranges.empty() || m_imported_logs_header.empty())
    {
        return;
    }

    std::size_t const expected_fields_count = m_imported_logs_header.size();
    std::unordered_map<std::size_t, std::vector<std::size_t>> log_id_to_view_indices;
    std::vector<SQLiteStorage::Range> requested_id_ranges;

    for (auto const& range : ranges)
    {
        auto const range_begin = std::min(range.begin, m_filtered_logs.size());
        auto const range_end = std::min(range.end, m_filtered_logs.size());
        if (range_begin >= range_end)
        {
            continue;
        }

        auto const first_log_id = m_filtered_logs[range_begin].log_id;
        auto const last_log_id = m_filtered_logs[range_end - 1].log_id;
        requested_id_ranges.push_back({.begin = first_log_id, .end = last_log_id + 1});

        for (std::size_t view_idx = range_begin; view_idx < range_end; ++view_idx)
        {
            auto const& filtered_item = m_filtered_logs[view_idx];

            auto& target_row = out_logs[view_idx];
            target_row.metadata = {
                .filter_id = filtered_item.filter_id,
                .highlight_id = filtered_item.highlight_filter_id};

            // Pre-allocate empty column slots matching the header size
            target_row.data.assign(expected_fields_count, "");

            std::size_t const log_id = filtered_item.log_id;
            log_id_to_view_indices[log_id].push_back(view_idx);
        }
    }

    if (requested_id_ranges.empty())
    {
        return;
    }

    std::unordered_map<std::size_t, std::vector<std::string>> line_buffer_pool;
    for (auto const& storage : m_sqlite_storages)
    {
        storage->ReadRowsByIDs(requested_id_ranges, line_buffer_pool);
    }

    for (auto const& [log_id, view_indices] : log_id_to_view_indices)
    {
        if (auto const it = line_buffer_pool.find(log_id); it != line_buffer_pool.end())
        {
            auto const& line = it->second;
            for (std::size_t const view_idx : view_indices)
            {
                if (!line.empty())
                {
                    out_logs[view_idx].data.assign(line.begin(), line.end());
                }
            }
        }
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V8
