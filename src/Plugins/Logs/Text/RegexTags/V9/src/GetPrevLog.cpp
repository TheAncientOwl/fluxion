/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetPrevLog.cpp
/// @author Alexandru Delegeanu
/// @version 9.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetPrevLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetPrevLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

bool RegexTags::GetPrevLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetPrevLogABI()");

    if (m_filtered_logs.empty() || !out_index)
    {
        return false;
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
                *out_index = i;
                return true;
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
            *out_index = i;
            return true;
        }
        if (i == 0)
        {
            break;
        }
    }

    return false;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
