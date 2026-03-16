/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Fluxion business logic entry point.
///

#pragma once

#include "Application/AppState.hpp"
#include "Core/Application/TGraphiteApplication.hpp"

namespace Fluxion::Application {

class FluxionApplication
    : public Graphite::Core::Application::TGraphiteApplication<Fluxion::Application::AppState>
{
public:
    using Graphite::Core::Application::TGraphiteApplication<AppState>::Ptr;

    ~FluxionApplication();

private:
    friend class Graphite::Core::Application::TGraphiteApplication<AppState>;
    FluxionApplication(
        Graphite::Core::Application::WindowConfiguration window_configuration,
        AppState initial_state);

private:
    void SetupFonts();

private:
    void AppInit() override;
};

} // namespace Fluxion::Application
