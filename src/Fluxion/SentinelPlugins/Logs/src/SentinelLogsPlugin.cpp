/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SentinelLogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Do nothing...
///

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

void SentinelLogsPlugin::OnEnable(Fluxion::API::LogsPlugin::Bridge::OnEnableData const& /*data*/)
{
    LOG_INFO("::OnEnable()");
}

void SentinelLogsPlugin::OnDisable(Fluxion::API::LogsPlugin::Bridge::OnDisableData const& /*data*/)
{
    LOG_INFO("::OnDisable()");
}

Bridge::ABI::StringView SentinelLogsPlugin::GetDisplayNameABI() const
{
    return "SentinelLogsPlugin";
}

Bridge::ABI::StringView SentinelLogsPlugin::GetDirectoryNameABI() const
{
    return "SentinelLogsPlugin";
}

void SentinelLogsPlugin::RenderMenu()
{
    LOG_INFO("::RenderMenu()");
}

void SentinelLogsPlugin::ImportLogsABI(Bridge::ABI::StringView const /*path*/)
{
    LOG_INFO("::ImportLogsABI()");
}

void SentinelLogsPlugin::ApplyFiltersABI(
    Bridge::ABI::Span<Bridge::Filter const> /* filters */,
    Bridge::ABI::Span<Bridge::Filter const> /* highlight_only */)
{
    LOG_SCOPE("::ApplyFiltersABI()");
}

void SentinelLogsPlugin::DisableFilters()
{
    LOG_SCOPE("::DisableFiltersABI()");
}

Bridge::ABI::Span<Bridge::ColumnDetails> SentinelLogsPlugin::GetTableHeaderABI() const
{
    LOG_SCOPE("::GetTableHeaderABI()");
    return {};
}

std::size_t SentinelLogsPlugin::GetTotalLogs() const
{
    LOG_SCOPE("::GetTotalLogs()");
    return 0;
}

void SentinelLogsPlugin::GetLogsABI(
    Bridge::ABI::Span<Bridge::Range> const /* ranges */,
    Bridge::ILogsWriter* /* out_logs */)
{
    LOG_SCOPE("::GetLogsABI()");
}

bool SentinelLogsPlugin::GetNextLogABI(
    Graphite::Common::Utility::UniqueID const& /* filter_id */,
    std::size_t const /* current_index */,
    std::size_t* /* out_index */)
{
    LOG_SCOPE("::GetNextLogABI()");
    return false;
}

bool SentinelLogsPlugin::GetPrevLogABI(
    Graphite::Common::Utility::UniqueID const& /* filter_id */,
    std::size_t const /* current_index */,
    std::size_t* /* out_index */)
{
    LOG_SCOPE("::GetPrevLogABI()");
    return false;
}

std::size_t SentinelLogsPlugin::GetLogsOperationTarget() const
{
    return 0;
}

std::size_t SentinelLogsPlugin::GetLogsOperationProgress() const
{
    return 0;
}

Fluxion::API::LogsPlugin::Bridge::ELogsOperationUnit SentinelLogsPlugin::GetLogsOperationUnit() const
{
    return API::LogsPlugin::Bridge::ELogsOperationUnit::Logs;
}

std::unique_ptr<Fluxion::API::LogsPlugin::IFluxionLogsPlugin> Create()
{
    return std::make_unique<Fluxion::SentinelPlugins::Logs::SentinelLogsPlugin>();
}

} // namespace Fluxion::SentinelPlugins::Logs
