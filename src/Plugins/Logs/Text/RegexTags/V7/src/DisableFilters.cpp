/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DisableFilters.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::DisableFilters);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::DisableFilters);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

void RegexTags::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::DisableFilters(): No logs imported before, nothing to disable");
        return;
    }

    m_logs_operation_progress = 0;
    m_logs_operation_target = m_total_logs_imported;

    m_filtered_logs.clear();
    for (std::size_t log_id = 0; log_id < m_total_logs_imported; ++log_id)
    {
        ++m_logs_operation_progress;
        m_filtered_logs.emplace_back(log_id);
    }

    LOG_INFO(
        "::DisableFilters(): Successfully disabled filters. Restored {} logs.",
        m_filtered_logs.size());

    m_logs_operation_progress = 0;
    m_logs_operation_target = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
