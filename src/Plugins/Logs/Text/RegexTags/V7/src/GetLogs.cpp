/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation @see RegexTags.hpp
///

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

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
    std::vector<std::size_t> requested_log_ids;

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

            // Pre-allocate empty column slots matching the header size
            target_row.data.assign(expected_fields_count, "");

            std::size_t const log_id = filtered_item.log_id;
            log_id_to_view_indices[log_id].push_back(view_idx);
            requested_log_ids.push_back(log_id);
        }
    }

    if (requested_log_ids.empty())
    {
        return;
    }

    std::sort(requested_log_ids.begin(), requested_log_ids.end());
    requested_log_ids.erase(
        std::unique(requested_log_ids.begin(), requested_log_ids.end()), requested_log_ids.end());

    std::vector<Scrolls::Scribe::Range> scroll_ranges;
    for (std::size_t const id : requested_log_ids)
    {
        if (scroll_ranges.empty() || id != scroll_ranges.back().end)
        {
            scroll_ranges.push_back({.begin = id, .end = id + 1});
        }
        else
        {
            scroll_ranges.back().end = id + 1;
        }
    }

    std::unordered_map<std::size_t, Scrolls::Papyrus::Line> line_buffer_pool{};

    m_scrolls.ReadRanges(
        scroll_ranges, [&line_buffer_pool](std::size_t const index) -> Scrolls::Papyrus::Line& {
            return line_buffer_pool[index];
        });

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

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
