/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetPrevLog.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

std::optional<std::size_t> RegexTags::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t current_index)
{
    LOG_SCOPE("::GetPrevLog()");

    if (!m_last_imported_logs_path)
    {
        LOG_INFO("::GetPrevLog(): No logs imported");
        return std::nullopt;
    }

    try
    {
        auto const filter_id_str = filter_id.ToString();
        auto const filtered_path = MakeFilteredLogsPath(*m_last_imported_logs_path);

        // First pass: find the last match before current_index
        {
            auto reader = CSV::Reader{filtered_path};
            std::optional<std::size_t> found_idx;
            for (auto row : reader)
            {
                if (row.empty())
                {
                    continue;
                }

                auto const row_num = reader.get_row_num() - 1; // Convert to 0-based

                if (row_num >= current_index)
                {
                    break;
                }

                if (row[0] == filter_id_str || row[1] == filter_id_str)
                {
                    found_idx = row_num;
                }
            }

            if (found_idx)
            {
                return found_idx;
            }
        }

        // Wrap around: search from current_index to end for last match
        {
            auto reader = CSV::Reader{filtered_path};
            std::optional<std::size_t> found_idx;
            for (auto row : reader)
            {
                if (row.empty())
                {
                    continue;
                }

                if (row[0] == filter_id_str || row[1] == filter_id_str)
                {
                    found_idx = reader.get_row_num() - 1; // Convert to 0-based
                }
            }
            return found_idx;
        }
    }
    catch (std::exception const& e)
    {
        LOG_WARN("::GetPrevLog(): Exception reading CSV: {}", e.what());
    }

    return std::nullopt;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
