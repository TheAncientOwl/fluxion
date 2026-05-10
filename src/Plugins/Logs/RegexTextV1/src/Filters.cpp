/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Filters.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

void RegexTextV1LogsPlugin::ApplyFilters(
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> /*filters*/,
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> /*highlight_only*/)
{
    LOG_SCOPE("::ApplyFilters()");
}

void RegexTextV1LogsPlugin::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1
