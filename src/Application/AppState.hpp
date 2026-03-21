/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.hpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Application state.
///

#pragma once

#include <memory>

#include "Api/Data.hpp"
#include "Api/IFluxionPlugin.hpp"

namespace Fluxion::Application {

struct AppState
{
    std::unique_ptr<Fluxion::API::IFluxionPlugin> logs_logic{nullptr};

    struct
    {
        Fluxion::API::Data::FiltersTabs tabs{};
        bool dirty{false};
    } filters{};

    struct
    {
        bool logs_view{true};
        bool debug{true};
        bool filters{true};
    } layers_active{};
};

} // namespace Fluxion::Application
