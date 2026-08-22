/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Progress.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V2/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V2 {

std::size_t RegexTags::GetTotalEstimatedImportLogs() const
{
    return m_total_import_logs;
}

std::size_t RegexTags::GetProcessedLogsProgress() const
{
    return m_logs_progress;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V2
