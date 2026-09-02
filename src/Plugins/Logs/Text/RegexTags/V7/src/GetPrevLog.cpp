/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetPrevLog.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetPrevLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetPrevLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

std::optional<std::size_t> RegexTags::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t current_index)
{
    LOG_SCOPE("::GetPrevLog()");

    if (m_filtered_logs.empty())
    {
        return std::nullopt;
    }

    auto matches = [&](Data::FilteredLog const& item) {
        return item.filter_id == filter_id || item.highlight_filter_id == filter_id;
    };

    // Backward search from current_index - 1
    if (current_index > 0)
    {
        for (std::size_t i = current_index - 1;; --i)
        {
            if (matches(m_filtered_logs[i]))
            {
                return i;
            }
            if (i == 0)
            {
                break;
            }
        }
    }

    // Wrap around to end
    for (std::size_t i = m_filtered_logs.size() - 1; i >= current_index; --i)
    {
        if (matches(m_filtered_logs[i]))
        {
            return i;
        }
        if (i == 0)
        {
            break;
        }
    }

    return std::nullopt;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
