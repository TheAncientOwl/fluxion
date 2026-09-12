/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 1.2.0
/// @brief Implementation @see RegexTags.hpp
///

#include <filesystem>
#include <span>
#include <system_error>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

void RegexTags::GetLogsABI(
    Fluxion::API::LogsPlugin::Bridge::ABI::Span<Fluxion::API::LogsPlugin::Bridge::Range> const ranges,
    Fluxion::API::LogsPlugin::Bridge::ILogsWriter* out_logs)
{
    LOG_SCOPE("::GetLogsABI()");

    std::span<Fluxion::API::LogsPlugin::Bridge::Range const> const ranges_span{ranges};

    std::stringstream ss{};
    for (auto const& range : ranges_span)
    {
        ss << "[" << range.begin << ", " << range.end << "), ";
    }
    LOG_INFO("::GetLogsABI(): Requested ranges: {}", ss.str());

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogsABI(): No logs were imported before");
        return;
    }

    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    if (!static_cast<bool>(total_logs_opt))
    {
        LOG_WARN("::GetLogsABI(): total_logs is not set in config");
        return;
    }

    auto const last_line_index{[&ranges_span]() {
        std::size_t last_idx{std::numeric_limits<std::size_t>::min()};
        for (auto const& range : ranges_span)
        {
            last_idx = std::max(last_idx, range.end);
        }
        return last_idx;
    }()};

    std::error_code ec;
    if (std::filesystem::file_size(*m_last_imported_logs_path, ec) == 0 || ec)
    {
        LOG_WARN(
            "::GetLogsABI(): File {} is currently 0 bytes or locked. Skipping read.",
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

        if (!std::any_of(ranges_span.begin(), ranges_span.end(), [row_num](auto const& range) {
                return range.begin <= row_num && row_num < range.end;
            }))
        {
            continue;
        }

        auto const actual_row_size{row.size() - 2}; // -2 = first 2 filter IDs
        std::vector<Fluxion::API::LogsPlugin::Bridge::ABI::StringView> columns;
        columns.reserve(actual_row_size);

        for (std::size_t col_idx = 2; col_idx < row.size(); ++col_idx)
        {
            columns.emplace_back(row[col_idx]);
        }

        out_logs->WriteData(row_num, columns);
        out_logs->WriteMetadata(
            row_num,
            Graphite::Common::Utility::UniqueID{row[0]},
            Graphite::Common::Utility::UniqueID{row[1]});
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
