/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTextV1LogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

std::string_view RegexTextV1LogsPlugin::GetDisplayName() const
{
    return "RegexTextV1";
}

void RegexTextV1LogsPlugin::OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& /*data*/) const
{
    LOG_SCOPE("::OnEnable()");
}

void RegexTextV1LogsPlugin::OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/) const
{
    LOG_SCOPE("::OnDisable()");
}

void RegexTextV1LogsPlugin::RenderMenu()
{
    LOG_SCOPE("::RenderMenu()");
}

void RegexTextV1LogsPlugin::ImportLogs(std::filesystem::path const& /*path*/)
{
    LOG_SCOPE("::ImportLogs()");
}

std::optional<std::size_t> RegexTextV1LogsPlugin::GetNextLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetNextLog()");
    return std::nullopt;
}

std::optional<std::size_t> RegexTextV1LogsPlugin::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetPrevLog()");
    return std::nullopt;
}

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

std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> RegexTextV1LogsPlugin::GetTableHeader() const
{
    return {};
}

std::size_t RegexTextV1LogsPlugin::GetTotalLogs() const
{
    return 0;
}

void RegexTextV1LogsPlugin::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& /*ranges*/,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter /*out_logs*/) const
{
    LOG_SCOPE("::GetLogs()");
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin()
{
    return new Fluxion::Plugins::Logs::RegexTextV1::RegexTextV1LogsPlugin();
}
