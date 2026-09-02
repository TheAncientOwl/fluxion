/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetNextLog.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetNextLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetNextLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

std::optional<std::size_t> RegexTags::GetNextLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index)
{
    LOG_SCOPE("::GetNextLog()");

    if (m_filtered_logs.empty())
    {
        return std::nullopt;
    }

    auto matches = [&](Data::FilteredLog const& item) {
        return item.filter_id == filter_id || item.highlight_filter_id == filter_id;
    };

    // Forward search from current_index + 1
    for (std::size_t i = current_index + 1; i < m_filtered_logs.size(); ++i)
    {
        if (matches(m_filtered_logs[i]))
        {
            return i;
        }
    }

    // Wrap around to start
    for (std::size_t i = 0; i <= current_index && i < m_filtered_logs.size(); ++i)
    {
        if (matches(m_filtered_logs[i]))
        {
            return i;
        }
    }

    return std::nullopt;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
