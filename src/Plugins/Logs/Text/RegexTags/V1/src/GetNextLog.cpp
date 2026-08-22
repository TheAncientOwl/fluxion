/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetNextLog.cpp
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

std::optional<std::size_t> RegexTags::GetNextLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index)
{
    LOG_SCOPE("::GetNextLog()");

    if (!m_last_imported_logs_path)
    {
        LOG_INFO("::GetNextLog(): No logs imported");
        return std::nullopt;
    }

    try
    {
        auto const filter_id_str = filter_id.ToString();
        auto const filtered_path = MakeFilteredLogsPath(*m_last_imported_logs_path);

        // First pass: forward search from current_index + 1
        {
            auto reader = CSV::Reader{filtered_path};
            for (auto row : reader)
            {
                if (row.empty())
                {
                    continue;
                }

                auto const row_num = reader.get_row_num() - 1; // Convert to 0-based

                if (row_num > current_index && (row[0] == filter_id_str || row[1] == filter_id_str))
                {
                    return row_num;
                }
            }
        }

        // Wrap around: search from beginning to current_index
        {
            auto reader = CSV::Reader{filtered_path};
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
                    return row_num;
                }
            }
        }
    }
    catch (std::exception const& e)
    {
        LOG_WARN("::GetNextLog(): Exception reading CSV: {}", e.what());
    }

    return std::nullopt;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
