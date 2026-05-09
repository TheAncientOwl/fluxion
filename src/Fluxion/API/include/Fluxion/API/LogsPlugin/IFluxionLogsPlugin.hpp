/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file IFluxionLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.12
/// @brief Plugin interface of Fluxion logs logic (parse/select/filter/...).
///

#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "Graphite/Common/Plugin/GraphiteExport.hpp"

#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"

namespace Fluxion::API::LogsPlugin {

/**
 * @brief Logi responsible with logs I/O, filtering, search, etc...
 *
 */
class IFluxionLogsPlugin
{
public:
    virtual std::string_view GetDisplayName() const = 0;

    virtual void OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data) = 0;
    virtual void OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& data) = 0;

    virtual void RenderMenu() = 0;

    virtual void ImportLogs(std::filesystem::path const& path) = 0;

    virtual std::optional<std::size_t> GetNextLog(Graphite::Common::Utility::UniqueID const& filter_id) = 0;
    virtual std::optional<std::size_t> GetPrevLog(Graphite::Common::Utility::UniqueID const& filter_id) = 0;

    /**
     * @brief Apply given filters.
     *
     * @param filters what logs are shown.
     * @param highlight_only override of @param filters for the colors.
     */
    virtual void ApplyFilters(
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters,
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> highlight_only) = 0;

    /**
     * @brief This should mark the filters as disabled.
     */
    virtual void DisableFilters() = 0;

    /**
     * @brief Get the Table Header.
     */
    virtual std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> GetTableHeader() const = 0;

    /**
     * @brief Get the Total Filtered Logs.
     */
    virtual std::size_t GetTotalLogs() const = 0;

    /**
     * @brief Request logs.
     *
     * @param ranges list of reequested chunks {begin inclusive, end exclusive}.
     * @param out_logs map<index, row> to be updated.
     *
     */
    virtual void GetLogs(
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) const = 0;

    virtual ~IFluxionLogsPlugin() = default;
};

} // namespace Fluxion::API::LogsPlugin

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
