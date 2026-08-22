/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation @see RegexTags.hpp
///

#include <filesystem>
#include <system_error>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

void RegexTags::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) const
{
    LOG_SCOPE("::GetLogs()");

    std::stringstream ss{};
    for (auto range : ranges)
    {
        ss << "[" << range.begin << ", " << range.end << "), ";
    }
    LOG_INFO("::GetLogs(): Requested ranges: {}", ss.str());

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
        for (auto& range : ranges)
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

        auto& target_row = out_logs[row_num];
        auto const actual_row_size{row.size() - 2}; // -2 = first 2 filter IDs
        if (target_row.data.size() != actual_row_size)
        {
            target_row.data.resize(actual_row_size);
        }

        for (std::size_t col_idx = 2; col_idx < row.size(); ++col_idx)
        {
            target_row.data[col_idx - 2] = std::move(row[col_idx]);
        }

        target_row.metadata = {
            .filter_id = Graphite::Common::Utility::UniqueID{row[0]},
            .highlight_id = Graphite::Common::Utility::UniqueID{row[1]}};
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
