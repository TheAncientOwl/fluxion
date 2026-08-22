/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetTotalLogs.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

std::size_t RegexTags::GetTotalLogs() const
{
    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    return static_cast<bool>(total_logs_opt) ? *total_logs_opt : 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
