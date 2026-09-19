/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file IFluxionLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Plugin interface of Fluxion logs logic (parse/select/filter/...).
///

#pragma once

#include <filesystem>
#include <optional>
#include <span>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp" // IWYU pragma: keep for register plugin macro
#include "Fluxion/API/LogsPlugin/Private/ABI/Adapter.hpp" // IWYU pragma: keep for register plugin macro
#include "Fluxion/API/LogsPlugin/Private/ABI/Safe.hpp"
#include "Fluxion/API/LogsPlugin/Private/ABI/Unsafe.hpp"
#include "Fluxion/API/LogsPlugin/Private/LogsPluginAPI.hpp" // IWYU pragma: keep for register plugin macro
#include "Graphite/Common/Plugin/GraphiteExport.hpp" // IWYU pragma: keep for register plugin macro

namespace Fluxion::API::LogsPlugin {

using UniqueID = Graphite::Common::Utility::UniqueID;

using OnEnableData = Private::ABI::Unsafe::OnEnableData;
using OnDisableData = Private::ABI::Unsafe::OnDisableData;

using Range = Private::ABI::Safe::Range;
using Filter = Private::ABI::Unsafe::Filter;
using ColumnDetails = Private::ABI::Unsafe::ColumnDetails;
using EConditionFlag = Private::ABI::Safe::EConditionFlag;

using LogRowData = Private::ABI::Safe::LogRowData;
using LogRowItem = Private::ABI::Safe::StringView;
using LogRowMetadata = Private::ABI::Safe::LogRowMetadata;
using OwningLogRow = Private::ABI::Unsafe::LogRow;
using OwningLogRowMetadata = Private::ABI::Unsafe::LogRowMetadata;
using ELogsOperationUnit = Private::ABI::Safe::ELogsOperationUnit;
using WriteLogRowDataFn = Private::ABI::Safe::WriteLogRowDataFn;
using WriteLogRowMetadataFn = Private::ABI::Safe::WriteLogRowMetadataFn;

namespace Adapter {

LogRowMetadata MakeMetadata(UniqueID const filter_id, UniqueID const highlight_filter_id) noexcept;

} // namespace Adapter

///
/// @brief Logic responsible with logs I/O, filtering, search, etc...
///
class IFluxionLogsPlugin
{
public:
    virtual ~IFluxionLogsPlugin() = default;

    virtual std::string_view GetDisplayName() const = 0;
    virtual std::string_view GetDirectoryName() const = 0;

    virtual void OnEnable(OnEnableData const& data) = 0;
    virtual void OnDisable(OnDisableData const& data) = 0;

    virtual void RenderMenu() = 0;

    virtual void ImportLogs(std::filesystem::path const& path) = 0;

    ///
    /// @brief Get the next log index relative to current item.
    ///
    /// @param filter_id the filter to search within.
    /// @param current_index current selected log index (0 if no item selected).
    ///
    virtual std::optional<std::size_t> GetNextLog(
        UniqueID const& filter_id,
        std::size_t const current_index = 0) = 0;

    ///
    /// @brief Get the previous log index relative to current item.
    ///
    /// @param filter_id the filter to search within.
    /// @param current_index current selected log index (0 if no item selected).
    ///
    virtual std::optional<std::size_t> GetPrevLog(
        UniqueID const& filter_id,
        std::size_t const current_index = 0) = 0;

    ///
    /// @brief Apply given filters.
    ///
    /// @param filters what logs are shown.
    /// @param highlight_only override of @param filters for the colors.
    ///
    virtual void ApplyFilters(
        std::span<Filter const> const filters,
        std::span<Filter const> const highlight_only) = 0;

    ///
    /// @brief This should mark the filters as disabled.
    ///
    virtual void DisableFilters() = 0;

    ///
    /// @brief Get the Table Header.
    ///
    virtual std::span<ColumnDetails const> GetTableHeader() const = 0;

    ///
    /// @brief Get the Total Filtered Logs.
    ///
    virtual std::size_t GetTotalLogs() const = 0;

    ///
    /// @brief Request logs.
    ///
    /// @param ranges list of reequested chunks {begin inclusive, end exclusive}.
    /// @param out_logs map<index, row> to be updated.
    ///
    ///
    virtual void GetLogs(
        std::span<Range const> const ranges,
        Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
        Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
        void* user_data) = 0;

    ///
    /// @brief Helper function to tell the total of logs in import file
    /// @note Should be thread safe
    ///
    virtual std::size_t GetLogsOperationTarget() const = 0;

    ///
    /// @brief Helper function to tell the unit in wwhich logs operation progress is measured
    /// @note Should be thread safe
    ///
    virtual ELogsOperationUnit GetLogsOperationUnit() const = 0;

    ///
    /// @brief Helper function to track how many logs were processed at call time
    /// @note Should be thread safe
    ///
    virtual std::size_t GetLogsOperationProgress() const = 0;
};

} // namespace Fluxion::API::LogsPlugin

#define FLUXION_REGISTER_LOGS_PLUGIN(PluginClass)                                                           \
    namespace {                                                                                             \
    inline Fluxion::API::LogsPlugin::Private::ABI::Safe::ColumnsDetailsSpan ConvertTableHeaderToSafeBuffer( \
        PluginClass const* plugin)                                                                          \
    {                                                                                                       \
        using namespace Fluxion::API::LogsPlugin::Private;                                                  \
        thread_local std::vector<ABI::Safe::ColumnDetails> safe_columns{};                                  \
        auto const columns = plugin->GetTableHeader();                                                      \
        safe_columns.clear();                                                                               \
        safe_columns.reserve(columns.size());                                                               \
        for (auto const& column : columns)                                                                  \
        {                                                                                                   \
            safe_columns.emplace_back(ABI::Adapter::ToSafe(column));                                        \
        }                                                                                                   \
        return ABI::Safe::ColumnsDetailsSpan{                                                               \
            .data = safe_columns.data(), .size = safe_columns.size()};                                      \
    }                                                                                                       \
    }                                                                                                       \
                                                                                                            \
    extern "C" GRAPHITE_EXPORT void CreateFluxionLogsPluginAPI(                                             \
        Fluxion::API::LogsPlugin::Private::LogsPluginAPI* out_api)                                          \
    {                                                                                                       \
        using Plugin = PluginClass;                                                                         \
        using API = Fluxion::API::LogsPlugin::Private::LogsPluginAPI;                                       \
        namespace ABI = Fluxion::API::LogsPlugin::Private::ABI;                                             \
        namespace Safe = Fluxion::API::LogsPlugin::Private::ABI::Safe;                                      \
        namespace Adapter = Fluxion::API::LogsPlugin::Private::ABI::Adapter;                                \
                                                                                                            \
        auto* plugin = new Plugin{};                                                                        \
                                                                                                            \
        *out_api = API{                                                                                     \
            .instance = plugin,                                                                             \
                                                                                                            \
            .Destroy = +[](void* instance) { delete static_cast<Plugin*>(instance); },                      \
                                                                                                            \
            .GetDisplayName =                                                                               \
                +[](void const* instance) {                                                                 \
                    auto const* plugin = static_cast<Plugin const*>(instance);                              \
                    return Adapter::ToSafe(plugin->GetDisplayName());                                       \
                },                                                                                          \
                                                                                                            \
            .GetDirectoryName =                                                                             \
                +[](void const* instance) {                                                                 \
                    auto const* plugin = static_cast<Plugin const*>(instance);                              \
                    return Adapter::ToSafe(plugin->GetDirectoryName());                                     \
                },                                                                                          \
                                                                                                            \
            .OnEnable =                                                                                     \
                +[](void* instance, Safe::OnEnableData const data) {                                        \
                    auto* plugin = static_cast<Plugin*>(instance);                                          \
                    auto const native_data = Adapter::ToNative(data);                                       \
                    plugin->OnEnable(native_data);                                                          \
                },                                                                                          \
                                                                                                            \
            .OnDisable =                                                                                    \
                +[](void* instance, Safe::OnDisableData const data) {                                       \
                    auto* plugin = static_cast<Plugin*>(instance);                                          \
                    auto const native_data = Adapter::ToNative(data);                                       \
                    plugin->OnDisable(native_data);                                                         \
                },                                                                                          \
                                                                                                            \
            .RenderMenu = +[](void* instance) { static_cast<Plugin*>(instance)->RenderMenu(); },            \
                                                                                                            \
            .ImportLogs =                                                                                   \
                +[](void* instance, Safe::StringView const path) {                                          \
                    auto* plugin = static_cast<Plugin*>(instance);                                          \
                    plugin->ImportLogs(std::filesystem::path{Adapter::ToNative(path)});                     \
                },                                                                                          \
                                                                                                            \
            .GetNextLog =                                                                                   \
                +[](void* instance,                                                                         \
                    Safe::UniqueID const filter_id,                                                         \
                    std::size_t const current_index,                                                        \
                    std::size_t* const out_index) {                                                         \
                    auto* plugin = static_cast<Plugin*>(instance);                                          \
                    auto const result =                                                                     \
                        plugin->GetNextLog(Adapter::ToNative(filter_id), current_index);                    \
                    if (!result)                                                                            \
                        return false;                                                                       \
                    *out_index = *result;                                                                   \
                    return true;                                                                            \
                },                                                                                          \
                                                                                                            \
            .GetPrevLog =                                                                                   \
                +[](void* instance,                                                                         \
                    Safe::UniqueID const filter_id,                                                         \
                    std::size_t const current_index,                                                        \
                    std::size_t* const out_index) {                                                         \
                    auto* plugin = static_cast<Plugin*>(instance);                                          \
                    auto const result =                                                                     \
                        plugin->GetPrevLog(Adapter::ToNative(filter_id), current_index);                    \
                    if (!result)                                                                            \
                        return false;                                                                       \
                    *out_index = *result;                                                                   \
                    return true;                                                                            \
                },                                                                                          \
                                                                                                            \
            .ApplyFilters =                                                                                 \
                +[](void* instance,                                                                         \
                    Safe::FiltersSpan const filters,                                                        \
                    Safe::FiltersSpan const highlight_only) {                                               \
                    auto* plugin = static_cast<Plugin*>(instance);                                          \
                    auto const native_filters_view = Adapter::ToNative(filters);                            \
                    auto const native_highlight_only_view = Adapter::ToNative(highlight_only);              \
                                                                                                            \
                    std::vector<ABI::Unsafe::Filter> native_filters;                                        \
                    native_filters.reserve(native_filters_view.size());                                     \
                    for (auto const& filter : native_filters_view)                                          \
                    {                                                                                       \
                        native_filters.emplace_back(Adapter::ToNative(filter));                             \
                    }                                                                                       \
                                                                                                            \
                    std::vector<ABI::Unsafe::Filter> native_highlight_only;                                 \
                    native_highlight_only.reserve(native_highlight_only_view.size());                       \
                    for (auto const& filter : native_highlight_only_view)                                   \
                    {                                                                                       \
                        native_highlight_only.emplace_back(Adapter::ToNative(filter));                      \
                    }                                                                                       \
                                                                                                            \
                    plugin->ApplyFilters(native_filters, native_highlight_only);                            \
                },                                                                                          \
                                                                                                            \
            .DisableFilters =                                                                               \
                +[](void* instance) { static_cast<Plugin*>(instance)->DisableFilters(); },                  \
                                                                                                            \
            .GetTableHeader =                                                                               \
                +[](void const* instance) {                                                                 \
                    auto const* plugin = static_cast<Plugin const*>(instance);                              \
                    return ConvertTableHeaderToSafeBuffer(plugin);                                          \
                },                                                                                          \
                                                                                                            \
            .GetTotalLogs =                                                                                 \
                +[](void const* instance) {                                                                 \
                    return static_cast<Plugin const*>(instance)->GetTotalLogs();                            \
                },                                                                                          \
                                                                                                            \
            .GetLogs =                                                                                      \
                +[](void* instance, Safe::RangesSpan const ranges, Safe::LogRowWriter writer) {             \
                    auto* plugin = static_cast<Plugin*>(instance);                                          \
                    plugin->GetLogs(                                                                        \
                        Adapter::ToNative(ranges),                                                          \
                        writer.write_data,                                                                  \
                        writer.write_metadata,                                                              \
                        writer.user_data);                                                                  \
                },                                                                                          \
                                                                                                            \
            .GetLogsOperationTarget =                                                                       \
                +[](void const* instance) {                                                                 \
                    return static_cast<Plugin const*>(instance)->GetLogsOperationTarget();                  \
                },                                                                                          \
                                                                                                            \
            .GetLogsOperationUnit =                                                                         \
                +[](void const* instance) {                                                                 \
                    return static_cast<Plugin const*>(instance)->GetLogsOperationUnit();                    \
                },                                                                                          \
                                                                                                            \
            .GetLogsOperationProgress =                                                                     \
                +[](void const* instance) {                                                                 \
                    return static_cast<Plugin const*>(instance)->GetLogsOperationProgress();                \
                },                                                                                          \
        };                                                                                                  \
    }
