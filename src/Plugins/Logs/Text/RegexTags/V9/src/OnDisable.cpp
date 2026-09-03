/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file OnDisable.cpp
/// @author Alexandru Delegeanu
/// @version 9.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

void RegexTags::OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/)
{
    LOG_SCOPE("::OnDisable()");
    LOG_TRACE("::OnDisable()");

    m_imported_logs_header.clear();
    m_filtered_logs.clear();
    m_sqlite_storages.clear();

    SaveSettings();

    SaveRegexTags(m_regex_tags.GetFront());
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
