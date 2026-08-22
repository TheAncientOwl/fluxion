/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DisableFilters.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V3/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3 {

void RegexTags::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
    auto settings{GetConfig()};
    auto const total_logs_imported_opt{settings.get<std::size_t>("total_logs_imported")};
    settings.set(
        "total_logs", static_cast<bool>(total_logs_imported_opt) ? *total_logs_imported_opt : 0);
    settings.Save();

    if (static_cast<bool>(m_last_imported_logs_path))
    {
        ImportLogs(*m_last_imported_logs_path);
    }
    else
    {
        LOG_INFO("::DisableFilters(): No logs imported before, nothing to disable");
    }
    m_logs_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3
