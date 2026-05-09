/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file main.cpp
/// @author Alexandru Delegeanu
/// @version 0.10
/// @brief ImGui entry point.
///

#include <stdlib.h>

#include "Fluxion/Fluxion.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Main);

int main()
{
    USE_LOG_SCOPE(Fluxion::Main);
    Graphite::Logger::GetLogger().LoadConfig();

    LOG_SCOPE("");

    Graphite::Application::WindowConfiguration window_configuration{};
    window_configuration.width = 950;
    window_configuration.height = 750;
    window_configuration.title = "Fluxion";

    auto app =
        Graphite::Application::TGraphiteApplication<Fluxion::Application::AppState, Fluxion::Application::EFluxionAction>::
            CreateApplication<Fluxion::Application::FluxionApplication>(
                std::move(window_configuration), Fluxion::Application::DefaultState::Make());

    app->Run();

    Graphite::Logger::GetLogger().SaveConfig();

    return EXIT_SUCCESS;
}
