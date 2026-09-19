/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 2.2
/// @brief Implementation @see RegexTags.hpp
///

#include <filesystem>
#include <system_error>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V2/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V2 {

void RegexTags::GetLogs(
    std::span<Fluxion::API::LogsPlugin::Range const> const ranges,
    Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
    Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
    void* user_data)
{
    LOG_SCOPE("::GetLogs()");

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogs(): No logs were imported before");
        return;
    }

    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    if (!static_cast<bool>(total_logs_opt))
    {
        LOG_WARN("::GetLogs(): total_logs is not set in config");
        return;
    }

    auto const last_line_index{[&ranges]() {
        std::size_t last_idx{std::numeric_limits<std::size_t>::min()};
        for (auto const& range : ranges)
        {
            last_idx = std::max(last_idx, range.end);
        }
        return last_idx;
    }()};

    std::error_code ec;
    if (std::filesystem::file_size(*m_last_imported_logs_path, ec) == 0 || ec)
    {
        LOG_WARN(
            "::GetLogs(): File {} is currently 0 bytes or locked. Skipping read.",
            m_last_imported_logs_path->string());
        return;
    }

    auto reader = CSV::Reader{MakeFilteredLogsPath(*m_last_imported_logs_path)};
    for (auto row : reader)
    {
        auto const row_num{reader.get_row_num() - 1};

        if (row_num > last_line_index || row_num > *total_logs_opt)
        {
            break;
        }

        if (!std::any_of(ranges.begin(), ranges.end(), [row_num](auto const& range) {
                return range.begin <= row_num && row_num < range.end;
            }))
        {
            continue;
        }

        // Prepare Row Data
        std::vector<Fluxion::API::LogsPlugin::LogRowItem> row_data{};
        auto const actual_row_size{row.size() - 2}; // -2 = first 2 filter IDs
        row_data.reserve(actual_row_size);

        for (std::size_t col_idx = 2; col_idx < row.size(); ++col_idx)
        {
            row_data.push_back({.data = row[col_idx].data(), .size = row[col_idx].size()});
        }

        Fluxion::API::LogsPlugin::LogRowData const safe_data{
            .data = row_data.data(), .size = row_data.size()};
        GRAPHITE_ASSERT(write_data != nullptr, "Received nullptr write_data function pointer");
        write_data(user_data, row_num, &safe_data);

        GRAPHITE_ASSERT(
            write_metadata != nullptr, "Received nullptr write_metadata function pointer");
        auto const metadata{Fluxion::API::LogsPlugin::Adapter::MakeMetadata(
            Fluxion::API::LogsPlugin::UniqueID(row[0]), Fluxion::API::LogsPlugin::UniqueID(row[1]))};
        write_metadata(user_data, row_num, &metadata);
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V2
