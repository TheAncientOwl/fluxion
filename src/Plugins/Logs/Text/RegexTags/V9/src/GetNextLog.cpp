/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetNextLog.cpp
/// @author Alexandru Delegeanu
/// @version 9.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetNextLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetNextLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

bool RegexTags::GetNextLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetNextLogABI()");

    if (m_filtered_logs.empty() || !out_index)
    {
        return false;
    }

    auto matches = [&](Data::FilteredLog const& item) {
        return item.filter_id == filter_id || item.highlight_filter_id == filter_id;
    };

    // Forward search from current_index + 1
    for (std::size_t i = current_index + 1; i < m_filtered_logs.size(); ++i)
    {
        if (matches(m_filtered_logs[i]))
        {
            *out_index = i;
            return true;
        }
    }

    // Wrap around to start
    for (std::size_t i = 0; i <= current_index && i < m_filtered_logs.size(); ++i)
    {
        if (matches(m_filtered_logs[i]))
        {
            *out_index = i;
            return true;
        }
    }

    return false;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
