/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 7.1
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
    std::span<Fluxion::API::LogsPlugin::Range const> const ranges,
    Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
    Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
    void* user_data)
{
    LOG_SCOPE("::GetLogs()");

    if (m_filtered_logs.empty() || ranges.empty() || m_imported_logs_header.empty())
    {
        return;
    }

    std::unordered_map<std::size_t, std::vector<std::size_t>> log_id_to_view_indices;
    std::vector<std::size_t> requested_log_ids;

    for (auto const& range : ranges)
    {
        for (std::size_t view_idx = range.begin;
             view_idx < range.end && view_idx < m_filtered_logs.size();
             ++view_idx)
        {
            auto const& filtered_item = m_filtered_logs[view_idx];

            GRAPHITE_ASSERT(
                write_metadata != nullptr, "Received nullptr write_metadata function pointer");
            auto const metadata{Fluxion::API::LogsPlugin::Adapter::MakeMetadata(
                filtered_item.filter_id, filtered_item.highlight_filter_id)};
            write_metadata(user_data, view_idx, &metadata);

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

            std::vector<Fluxion::API::LogsPlugin::LogRowItem> row_data{};
            row_data.reserve(line.size());

            for (auto const& field : line)
            {
                row_data.push_back({.data = field.data(), .size = field.size()});
            }

            Fluxion::API::LogsPlugin::LogRowData const safe_data{
                .data = row_data.data(), .size = row_data.size()};
            for (std::size_t const view_idx : view_indices)
            {
                GRAPHITE_ASSERT(
                    write_data != nullptr, "Received nullptr write_data function pointer");
                write_data(user_data, view_idx, &safe_data);
            }
        }
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
