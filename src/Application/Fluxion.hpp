/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Fluxion business logic entry point.
///

#pragma once

#include "AppState.hpp"
#include "Graphite/Application/TGraphiteApplication.hpp"

namespace Fluxion::Application {

class FluxionApplication
    : public Graphite::Application::TGraphiteApplication<Fluxion::Application::AppState>
{
public:
    using Ptr = Graphite::Application::TGraphiteApplication<AppState>::Ptr;

    ~FluxionApplication();

private:
    friend class Graphite::Application::TGraphiteApplication<AppState>;
    FluxionApplication(Graphite::Application::WindowConfiguration window_configuration, AppState initial_state);

private:
    void SetupFonts();

private:
    void AppInit() override;
};

} // namespace Fluxion::Application
