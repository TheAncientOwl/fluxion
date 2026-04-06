/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file IFluxionPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.9
/// @brief Plugin interface of Fluxion logs logic (parse/select/filter/...).
///

#pragma once

#include <filesystem>
#include <vector>

#include "Graphite/Common/Plugin/GraphiteExport.hpp"

#include "Fluxion/API/Data.hpp"

namespace Fluxion::API {

class IFluxionPlugin
{
public:
    virtual void OnEnable(Fluxion::API::Data::Plugin::OnEnableData const& data) const = 0;
    virtual void OnDisable(Fluxion::API::Data::Plugin::OnDisableData const& data) const = 0;

    virtual std::string_view GetDisplayName() const = 0;
    virtual void RenderMenuLayer() = 0;

    virtual void ImportLogs(std::filesystem::path const& path) = 0;
    virtual void ApplyFilters(
        std::vector<Fluxion::API::Data::Filters::Active::Filter> filters,
        std::vector<Fluxion::API::Data::Filters::Active::Filter> highlight_only) = 0;
    virtual void DisableFilters() = 0;

    virtual std::vector<Fluxion::API::Data::Logs::ColumnDetails> GetTableHeader() const = 0;
    virtual std::size_t GetTotalLogs() const = 0;

    /**
     * @brief Request logs
     *
     * @param ranges list of reequested chunks {begin inclusive, end exclusive}
     * @param out_logs map<index, row> to be updated
     *
     */
    virtual void GetLogs(
        std::vector<Fluxion::API::Data::Logs::Range> const& ranges,
        Fluxion::API::Data::Logs::IndexToLogRowMapWriter out_logs) const = 0;

    virtual ~IFluxionPlugin() = default;
};

} // namespace Fluxion::API

extern "C" GRAPHITE_EXPORT Fluxion::API::IFluxionPlugin* CreateFluxionPlugin();
typedef Fluxion::API::IFluxionPlugin* (*CreateFluxionPluginFactory)();
