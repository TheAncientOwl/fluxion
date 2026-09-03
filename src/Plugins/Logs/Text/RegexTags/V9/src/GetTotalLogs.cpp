/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetTotalLogs.cpp
/// @author Alexandru Delegeanu
/// @version 9.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetTotalLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::GetTotalLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

std::size_t RegexTags::GetTotalLogs() const
{
    return m_filtered_logs.size();
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
