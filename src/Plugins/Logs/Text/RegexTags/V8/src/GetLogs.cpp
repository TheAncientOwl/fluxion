/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 8.1
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

            GRAPHITE_ASSERT(
                write_metadata != nullptr, "Received nullptr write_metadata function pointer");
            auto const metadata{Fluxion::API::LogsPlugin::Adapter::MakeMetadata(
                filtered_item.filter_id, filtered_item.highlight_filter_id)};
            write_metadata(user_data, view_idx, &metadata);

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

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V8
