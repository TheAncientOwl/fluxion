/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file IFluxionPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.7
/// @brief Plugin interface of Fluxion logs logic (parse/select/filter/...).
///

#pragma once

#include <filesystem>
#include <string>
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
    virtual void ApplyFilters(std::vector<std::shared_ptr<Fluxion::API::Data::FiltersTab>> const& tabs) = 0;

    virtual std::vector<Fluxion::API::Data::LogsTableColumnDetails> GetTableHeader() const = 0;
    virtual std::size_t GetTotalLogs() const = 0;

    /**
     * @brief Get the Logs Chunk object
     *
     * @param begin inclusive
     * @param end exclusive
     * @param out vector<vector<string>> with memory already allocated
     *
     * @return how many rows were filled
     */
    virtual std::size_t GetLogsChunk(
        std::size_t const begin,
        std::size_t const end,
        std::vector<std::vector<std::string>>& out) const = 0;

    virtual ~IFluxionPlugin() = default;
};

} // namespace Fluxion::API

extern "C" GRAPHITE_EXPORT Fluxion::API::IFluxionPlugin* CreateFluxionPlugin();
typedef Fluxion::API::IFluxionPlugin* (*CreateFluxionPluginFactory)();
