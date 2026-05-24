/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DisableFilters.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

void RegexTags::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
    if (static_cast<bool>(m_last_imported_logs_path))
    {
        ImportLogs(*m_last_imported_logs_path);
    }
    else
    {
        LOG_INFO("::DisableFilters(): No logs imported before, nothing to disable");
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
