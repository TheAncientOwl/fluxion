/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 2.2.0
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

void RegexTags::GetLogsABI(Bridge::ABI::Span<Bridge::Range> const _ranges, Bridge::ILogsWriter* out_logs)
{
    LOG_SCOPE("::GetLogs()");

    std::span<Bridge::Range const> const ranges{_ranges};

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

        if (row_num > last_line_index || row_num >= *total_logs_opt)
        {
            break;
        }

        if (!std::any_of(ranges.begin(), ranges.end(), [row_num](auto const& range) {
                return range.begin <= row_num && row_num < range.end;
            }))
        {
            continue;
        }

        std::vector<Bridge::ABI::StringView> columns;
        columns.reserve(row.size() > 2 ? row.size() - 2 : 0);
        for (std::size_t col_idx = 2; col_idx < row.size(); ++col_idx)
        {
            columns.emplace_back(row[col_idx]);
        }

        out_logs->WriteData(row_num, columns);

        Graphite::Common::Utility::UniqueID const filter_id{row[0]};
        Graphite::Common::Utility::UniqueID const highlight_id{row[1]};
        out_logs->WriteMetadata(row_num, filter_id, highlight_id);
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V2
