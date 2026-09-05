/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 9.1
/// @brief Implementation @see RegexTags.hpp
///

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

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
    std::unordered_map<std::size_t, std::vector<std::string>*> log_id_to_output;
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

            // Reset column values while preserving vector and string capacity.
            target_row.data.resize(expected_fields_count);
            for (auto& value : target_row.data)
            {
                value.clear();
            }

            std::size_t const log_id = filtered_item.log_id;
            log_id_to_output[log_id] = &target_row.data;
        }
    }

    if (requested_id_ranges.empty())
    {
        return;
    }

    for (auto const& storage : m_sqlite_storages)
    {
        storage->ReadRowsByIDsInto(requested_id_ranges, log_id_to_output);
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
