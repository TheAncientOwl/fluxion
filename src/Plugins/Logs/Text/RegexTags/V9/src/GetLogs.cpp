/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 9.6
/// @brief Implementation @see RegexTags.hpp
///

#include <algorithm>
#include <limits>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

void RegexTags::GetLogsABI(Bridge::ABI::Span<Bridge::Range> const ranges, Bridge::ILogsWriter* out_logs)
{
    LOG_SCOPE("::GetLogsABI()");

    std::span<Bridge::Range const> ranges_span{ranges};

    if (m_filtered_logs.empty() || ranges_span.empty() || m_imported_logs_header.empty() || !out_logs)
    {
        return;
    }

    std::size_t const expected_fields_count = m_imported_logs_header.size();

    struct LogOutputTarget
    {
        std::size_t view_idx;
        std::size_t log_id;
        std::vector<std::string> data;
    };

    std::vector<LogOutputTarget> targets;
    std::vector<SQLiteStorage::Range> requested_id_ranges;

    for (auto const& range : ranges_span)
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

            // Stream metadata directly via WriteMetadata
            out_logs->WriteMetadata(
                view_idx, filtered_item.filter_id, filtered_item.highlight_filter_id);

            targets.push_back(
                {.view_idx = view_idx,
                 .log_id = filtered_item.log_id,
                 .data = std::vector<std::string>(expected_fields_count)});
        }
    }

    if (requested_id_ranges.empty() || targets.empty())
    {
        return;
    }

    std::vector<std::pair<std::size_t, std::vector<std::string>*>> log_id_to_output;
    log_id_to_output.reserve(targets.size());
    for (auto& target : targets)
    {
        log_id_to_output.emplace_back(target.log_id, &target.data);
    }

    std::sort(log_id_to_output.begin(), log_id_to_output.end(), [](auto const& lhs, auto const& rhs) {
        return lhs.first < rhs.first;
    });

    for (auto const& storage : m_sqlite_storages)
    {
        auto const shard_begin = storage->GetIDOffset();
        auto const shard_size = storage->GetWrittenRows();
        auto const shard_end = shard_size > std::numeric_limits<std::size_t>::max() - shard_begin
                                   ? std::numeric_limits<std::size_t>::max()
                                   : shard_begin + shard_size;

        std::vector<SQLiteStorage::Range> shard_id_ranges{};
        shard_id_ranges.reserve(requested_id_ranges.size());
        for (auto const& requested_range : requested_id_ranges)
        {
            auto const begin = std::max(requested_range.begin, shard_begin);
            auto const end = std::min(requested_range.end, shard_end);
            if (begin < end)
            {
                shard_id_ranges.push_back({.begin = begin, .end = end});
            }
        }

        if (!shard_id_ranges.empty())
        {
            storage->ReadRowsByIDsInto(shard_id_ranges, log_id_to_output);
        }
    }

    // Stream row columns back through WriteData
    for (auto const& target : targets)
    {
        std::vector<Bridge::ABI::StringView> string_views;
        string_views.reserve(target.data.size());
        for (auto const& val : target.data)
        {
            string_views.emplace_back(val);
        }

        out_logs->WriteData(target.view_idx, string_views);
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
