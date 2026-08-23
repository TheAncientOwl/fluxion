/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetPrevLog.cpp
/// @author Alexandru Delegeanu
/// @version 3.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V3/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3 {

std::optional<std::size_t> RegexTags::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t current_index)
{
    LOG_SCOPE("::GetPrevLog()");

    if (!m_db_reader.has_value())
    {
        LOG_INFO("::GetPrevLog(): Database reader not initialized");
        return std::nullopt;
    }

    return m_db_reader->GetPrevFilteredIndex(filter_id.ToString(), current_index);
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3
