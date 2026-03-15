/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file main.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief ImGui entry point.
///

#include <stdlib.h>

#include "imgui/imgui.h"

#include "Core/Logger/Logger.hpp"

int main()
{
    LOG_SCOPE("");

    // Graphite::Core::Application::WindowConfiguration window_configuration{};
    // window_configuration.width = 500;
    // window_configuration.height = 600;
    // window_configuration.title = "Players Manager";

    // auto app =
    //     Graphite::Core::Application::TGraphiteApplication<Graphite::Project::AppState>::CreateApplication<
    //         Graphite::Project::PlayersApplication>(
    //         std::move(window_configuration), Graphite::Project::AppState{});

    // app->Run();

    return EXIT_SUCCESS;
}
