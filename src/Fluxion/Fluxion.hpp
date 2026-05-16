/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.hpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief Fluxion business logic entry point.
///

#pragma once

#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/TGraphiteApplication.hpp"

namespace Fluxion::Application {

class FluxionApplication : public Graphite::Application::TGraphiteApplication<AppState, EFluxionAction>
{
public:
    using Ptr = Graphite::Application::TGraphiteApplication<AppState, EFluxionAction>::Ptr;

    ~FluxionApplication();

private:
    friend class Graphite::Application::TGraphiteApplication<AppState, EFluxionAction>;
    FluxionApplication(Graphite::Application::WindowConfiguration window_configuration, AppState initial_state);

private:
    void SetupFonts();

private:
    void OnInit() override;
    void OnShutdown() override;
    void OnProcessAction(Graphite::Common::Utility::TAppAction<EFluxionAction> const& action) override;
};

} // namespace Fluxion::Application
