/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SentinelLogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Do nothing...
///

#include <filesystem>
#include <string_view>
#include <vector>

#include "Fluxion/SentinelPlugins/Logs/SentinelLogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::SentinelPlugins::Logs);
USE_LOG_SCOPE(Fluxion::SentinelPlugins::Logs);

namespace Fluxion::SentinelPlugins::Logs {

SentinelLogsPlugin::SentinelLogsPlugin()
{
    LOG_INFO("::SentinelLogsPlugin()");
}

void SentinelLogsPlugin::OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& /*data*/)
{
    LOG_INFO("::OnEnable()");
}

void SentinelLogsPlugin::OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/)
{
    LOG_INFO("::OnDisable()");
}

std::string_view SentinelLogsPlugin::GetDisplayName() const
{
    return "SentinelLogsPlugin";
}

void SentinelLogsPlugin::RenderMenu()
{
    LOG_INFO("::RenderMenu()");
}

void SentinelLogsPlugin::ImportLogs(std::filesystem::path const& /*path*/)
{
    LOG_INFO("::ImportLogs()");
}

void SentinelLogsPlugin::ApplyFilters(
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> /*_filters*/,
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> /*_highlight_only*/)
{
    LOG_SCOPE("::ApplyFilters()");
}

void SentinelLogsPlugin::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
}

std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> SentinelLogsPlugin::GetTableHeader() const
{
    LOG_SCOPE("::GetTableHeader()");
    return {};
}

std::size_t SentinelLogsPlugin::GetTotalLogs() const
{
    LOG_SCOPE("::GetTotalLogs()");
    return 0;
}

void SentinelLogsPlugin::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& /*ranges*/,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter /*out_logs*/) const
{
    LOG_SCOPE("::GetLogs()");
}

std::optional<std::size_t> SentinelLogsPlugin::GetNextLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetNextLog()");
    return std::nullopt;
}

std::optional<std::size_t> SentinelLogsPlugin::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetPrevLog()");
    return std::nullopt;
}

std::unique_ptr<Fluxion::API::LogsPlugin::IFluxionLogsPlugin> Create()
{
    return std::make_unique<Fluxion::SentinelPlugins::Logs::SentinelLogsPlugin>();
}

} // namespace Fluxion::SentinelPlugins::Logs
