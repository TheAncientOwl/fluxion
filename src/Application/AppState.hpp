/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.hpp
/// @author Alexandru Delegeanu
/// @version 0.8
/// @brief Application state.
///

#pragma once

#include <memory>

#include "Fluxion/API/Data.hpp"
#include "Fluxion/API/IFluxionPlugin.hpp"

namespace Fluxion::Application {

enum class EFluxionAction : std::uint8_t
{
    None = 0,
    FilterAction = 1
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
};

namespace DefaultState {

AppState Make();

Fluxion::API::Data::FiltersTabs::StorageType MakeDefaultTabs();

} // namespace DefaultState

} // namespace Fluxion::Application
