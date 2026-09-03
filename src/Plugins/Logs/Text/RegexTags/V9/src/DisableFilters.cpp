/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DisableFilters.cpp
/// @author Alexandru Delegeanu
/// @version 9.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::DisableFilters);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::DisableFilters);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

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
    m_filtered_logs.reserve(m_total_logs_imported);
    for (auto const& storage : m_sqlite_storages)
    {
        auto const count = storage->GetWrittenRows();
        auto const offset = storage->GetIDOffset();
        for (std::size_t index = 0; index < count; ++index)
        {
            ++m_logs_operation_progress;
            m_filtered_logs.emplace_back(offset + index);
        }
    }

    LOG_INFO(
        "::DisableFilters(): Successfully disabled filters. Restored {} logs.",
        m_filtered_logs.size());

    m_logs_operation_progress = 0;
    m_logs_operation_target = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
