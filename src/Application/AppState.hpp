/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.hpp
/// @author Alexandru Delegeanu
/// @version 0.11
/// @brief Application state.
///

#pragma once

#include <memory>

#include "Fluxion/API/Data.hpp"
#include "Fluxion/API/IFluxionPlugin.hpp"
#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"

namespace Fluxion::Application {

enum class EFluxionAction : std::uint8_t
{
    None = 0,
    FilterAction = 1,
    LogsViewLayerAction = 2
};

struct VisibleLogsChunk
{
    std::vector<std::vector<std::string>> data{};
    std::size_t filled_size{};
};

struct AppState
{
    std::unique_ptr<Fluxion::API::IFluxionPlugin> logs_logic{nullptr};

    struct
    {
        Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<Fluxion::API::Data::FiltersTab::Ptr>>
            tabs{};
    } filters{};

    struct
    {
        bool logs_view{true};
        bool debug{true};
        bool filters{true};
    } layers_active{};

    struct
    {
        Graphite::Common::DataStructures::TSwapDoubleBuffer<VisibleLogsChunk> visible_chunk{};
        std::vector<Fluxion::API::Data::LogsTableColumnDetails> table_header{};
    } logs{};
};

namespace DefaultState {

AppState Make();

std::vector<Fluxion::API::Data::FiltersTab::Ptr> MakeDefaultTabs();

} // namespace DefaultState

} // namespace Fluxion::Application
