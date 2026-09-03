/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file OnDisable.cpp
/// @author Alexandru Delegeanu
/// @version 8.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V8/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V8 {

void RegexTags::OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/)
{
    LOG_SCOPE("::OnDisable()");
    LOG_TRACE("::OnDisable()");

    SaveSettings();

    SaveRegexTags(m_regex_tags.GetFront());
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V8
