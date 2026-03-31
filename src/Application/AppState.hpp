/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.hpp
/// @author Alexandru Delegeanu
/// @version 0.9
/// @brief Application state.
///

#pragma once

#include <memory>

#include "Fluxion/API/Data.hpp"
#include "Fluxion/API/IFluxionPlugin.hpp"
#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/DataStructures/TDoubleBufferedVector.hpp"

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
        Fluxion::API::Data::FiltersTabs tabs{};
    } filters{};

    struct
    {
        bool logs_view{true};
        bool debug{true};
        bool filters{true};
    } layers_active{};

    struct
    {
        Graphite::Common::DataStructures::TDoubleBuffer<VisibleLogsChunk> visible_chunk{};
        std::vector<std::string> table_header{}; // todo: replace string with proper struct
    } logs{};
};

namespace DefaultState {

AppState Make();

Fluxion::API::Data::FiltersTabs::StorageType MakeDefaultTabs();

} // namespace DefaultState

} // namespace Fluxion::Application
