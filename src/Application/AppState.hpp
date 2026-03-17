/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
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

    bool logs_view_open{true};

    struct
    {
        Fluxion::API::Data::FilterTabs tabs{};
        bool menu_open{true};
    } filters{};
};

} // namespace Fluxion::Application
