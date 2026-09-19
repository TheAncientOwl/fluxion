/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SentinelLogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Do nothing...
///

#include "Fluxion/SentinelPlugins/Logs/SentinelLogsPlugin.hpp"
#include "Fluxion/API/LogsPlugin/Private/ABI/Adapter.hpp"
#include "Fluxion/API/LogsPlugin/Private/ABI/Safe.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::SentinelLogsPlugin);
USE_LOG_SCOPE(Fluxion::SentinelLogsPlugin);

namespace Fluxion::SentinelLogsPlugin {

namespace ABI = Fluxion::API::LogsPlugin::Private::ABI;

SentinelLogsPlugin::SentinelLogsPlugin()
{
    LOG_INFO("::SentinelLogsPlugin()");
}

void SentinelLogsPlugin::OnEnable(Fluxion::API::LogsPlugin::OnEnableData const& /*data*/)
{
    LOG_INFO("::OnEnable()");
}

void SentinelLogsPlugin::OnDisable(Fluxion::API::LogsPlugin::OnDisableData const& /*data*/)
{
    LOG_INFO("::OnDisable()");
}

std::string_view SentinelLogsPlugin::GetDisplayName() const
{
    return "SentinelLogsPlugin";
}

std::string_view SentinelLogsPlugin::GetDirectoryName() const
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
    std::span<ABI::Unsafe::Filter const> const /*_filters*/,
    std::span<ABI::Unsafe::Filter const> const /*_highlight_only*/)
{
    LOG_SCOPE("::ApplyFilters()");
}

void SentinelLogsPlugin::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
}

std::span<Fluxion::API::LogsPlugin::ColumnDetails const> SentinelLogsPlugin::GetTableHeader() const
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
    std::span<ABI::Safe::Range const> const /* ranges */,
    Fluxion::API::LogsPlugin::WriteLogRowDataFn /* write_data */,
    Fluxion::API::LogsPlugin::WriteLogRowMetadataFn /* write_metadata */,
    void* /* user_data */)
{
    LOG_SCOPE("::GetLogs()");
}

std::optional<std::size_t> SentinelLogsPlugin::GetNextLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/,
    std::size_t const /*current_index*/)
{
    LOG_SCOPE("::GetNextLog()");
    return std::nullopt;
}

std::optional<std::size_t> SentinelLogsPlugin::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/,
    std::size_t const /*current_index*/)
{
    LOG_SCOPE("::GetPrevLog()");
    return std::nullopt;
}

std::size_t SentinelLogsPlugin::GetLogsOperationTarget() const
{
    return 0;
}

std::size_t SentinelLogsPlugin::GetLogsOperationProgress() const
{
    return 0;
}

ABI::Safe::ELogsOperationUnit SentinelLogsPlugin::GetLogsOperationUnit() const
{
    return ABI::Safe::ELogsOperationUnit::Logs;
}

Fluxion::API::LogsPlugin::Private::LogsPluginAPI Create()
{
    using Plugin = Fluxion::SentinelLogsPlugin::SentinelLogsPlugin;
    using API = Fluxion::API::LogsPlugin::Private::LogsPluginAPI;
    namespace Safe = Fluxion::API::LogsPlugin::Private::ABI::Safe;
    namespace Adapter = Fluxion::API::LogsPlugin::Private::ABI::Adapter;

    auto* plugin = new Plugin{};

    return API{
        .instance = plugin,

        .Destroy = +[](void* instance) { delete static_cast<Plugin*>(instance); },

        .GetDisplayName =
            +[](void const* instance) {
                auto const* plugin = static_cast<Plugin const*>(instance);
                return Adapter::ToSafe(plugin->GetDisplayName());
            },

        .GetDirectoryName =
            +[](void const* instance) {
                auto const* plugin = static_cast<Plugin const*>(instance);
                return Adapter::ToSafe(plugin->GetDirectoryName());
            },

        .OnEnable =
            +[](void* instance, Safe::OnEnableData const data) {
                auto* plugin = static_cast<Plugin*>(instance);
                plugin->OnEnable(Adapter::ToNative(data));
            },

        .OnDisable =
            +[](void* instance, Safe::OnDisableData const data) {
                auto* plugin = static_cast<Plugin*>(instance);
                plugin->OnDisable(Adapter::ToNative(data));
            },

        .RenderMenu = +[](void* instance) { static_cast<Plugin*>(instance)->RenderMenu(); },

        .ImportLogs =
            +[](void* instance, Safe::StringView const path) {
                auto* plugin = static_cast<Plugin*>(instance);
                plugin->ImportLogs(std::filesystem::path{Adapter::ToNative(path)});
            },

        .GetNextLog =
            +[](void* instance,
                Safe::UniqueID const filter_id,
                std::size_t const current_index,
                std::size_t* const out_index) {
                auto* plugin = static_cast<Plugin*>(instance);

                auto const result = plugin->GetNextLog(Adapter::ToNative(filter_id), current_index);

                if (!result)
                    return false;

                *out_index = *result;
                return true;
            },

        .GetPrevLog =
            +[](void* instance,
                Safe::UniqueID const filter_id,
                std::size_t const current_index,
                std::size_t* const out_index) {
                auto* plugin = static_cast<Plugin*>(instance);

                auto const result = plugin->GetPrevLog(Adapter::ToNative(filter_id), current_index);

                if (!result)
                    return false;

                *out_index = *result;
                return true;
            },

        .ApplyFilters =
            +[](void* instance, Safe::FiltersSpan const filters, Safe::FiltersSpan const highlight_only) {
                auto* plugin = static_cast<Plugin*>(instance);

                auto const native_filters_view = Adapter::ToNative(filters);
                auto const native_highlight_only_view = Adapter::ToNative(highlight_only);

                std::vector<ABI::Unsafe::Filter> native_filters;
                native_filters.reserve(native_filters_view.size());

                for (auto const& filter : native_filters_view)
                    native_filters.emplace_back(Adapter::ToNative(filter));

                std::vector<ABI::Unsafe::Filter> native_highlight_only;
                native_highlight_only.reserve(native_highlight_only_view.size());

                for (auto const& filter : native_highlight_only_view)
                    native_highlight_only.emplace_back(Adapter::ToNative(filter));

                plugin->ApplyFilters(native_filters, native_highlight_only);
            },

        .DisableFilters = +[](void* instance) { static_cast<Plugin*>(instance)->DisableFilters(); },

        .GetTableHeader =
            +[](void const* instance) {
                auto const* plugin = static_cast<Plugin const*>(instance);
                auto const columns = plugin->GetTableHeader();

                static thread_local std::vector<Safe::ColumnDetails> safe_columns{};

                safe_columns.clear();
                safe_columns.reserve(columns.size());

                for (auto const& column : columns)
                    safe_columns.emplace_back(Adapter::ToSafe(column));

                return Safe::ColumnsDetailsSpan{
                    .data = safe_columns.data(), .size = safe_columns.size()};
            },

        .GetTotalLogs =
            +[](void const* instance) {
                return static_cast<Plugin const*>(instance)->GetTotalLogs();
            },

        .GetLogs =
            +[](void* instance, Safe::RangesSpan const ranges, Safe::LogRowWriter writer) {
                auto* plugin = static_cast<Plugin*>(instance);

                plugin->GetLogs(
                    Adapter::ToNative(ranges), writer.write_data, writer.write_metadata, writer.user_data);
            },

        .GetLogsOperationTarget =
            +[](void const* instance) {
                return static_cast<Plugin const*>(instance)->GetLogsOperationTarget();
            },

        .GetLogsOperationUnit =
            +[](void const* instance) {
                return static_cast<Plugin const*>(instance)->GetLogsOperationUnit();
            },

        .GetLogsOperationProgress =
            +[](void const* instance) {
                return static_cast<Plugin const*>(instance)->GetLogsOperationProgress();
            },
    };
}

} // namespace Fluxion::SentinelLogsPlugin
