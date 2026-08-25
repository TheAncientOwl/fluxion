/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Progress.cpp
/// @author Alexandru Delegeanu
/// @version 5.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V5/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5 {

std::size_t RegexTags::GetLogsOperationTarget() const
{
    return m_logs_operation_target;
}

std::size_t RegexTags::GetLogsOperationProgress() const
{
    return m_logs_operation_progress;
}

Fluxion::API::LogsPlugin::Data::ELogsOperationUnit RegexTags::GetLogsOperationUnit() const
{
    return m_logs_operation_unit;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5
