/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file IFluxionPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Plugin interface of Fluxion logs logic (parse/select/filter/...).
///

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Graphite/Common/GraphiteExport.hpp"

#include "Data.hpp"

namespace Fluxion::API {

class IFluxionPlugin
{
public:
    virtual void OnEnable(Fluxion::API::Data::Plugin::OnEnableData const& data) const = 0;
    virtual void OnDisable(Fluxion::API::Data::Plugin::OnDisableData const& data) const = 0;

    virtual std::string_view GetDisplayName() const = 0;
    virtual void RenderMenuLayer() = 0;

    virtual void ImportLogs(std::filesystem::path const& path) = 0;
    virtual void ApplyFilters(Fluxion::API::Data::FiltersTabs const& tabs) = 0;

    virtual Fluxion::API::Data::LogsTableHeader GetTableHeader() const = 0;
    virtual std::size_t GetTotalLogs() const = 0;
    virtual Fluxion::API::Data::LogsChunk GetLogsChunk(std::size_t const begin, std::size_t const end)
        const = 0;

    virtual ~IFluxionPlugin() = default;
};

} // namespace Fluxion::API

extern "C" GRAPHITE_EXPORT Fluxion::API::IFluxionPlugin* CreateFluxionPlugin();
typedef Fluxion::API::IFluxionPlugin* (*CreateFluxionPluginFactory)();
