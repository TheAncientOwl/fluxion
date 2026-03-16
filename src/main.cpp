/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file main.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief ImGui entry point.
///

#include <stdlib.h>

#include "Application/Fluxion.hpp"
#include "Core/Logger/Logger.hpp"

Fluxion::Application::AppState MakeDefaultAppState()
{
    Fluxion::Application::AppState app_state{};

    using namespace Fluxion::API::Data;
    using UniqueID = Graphite::Core::Common::UniqueID;

    auto& filter_tabs{app_state.filters.data};

    {
        auto component = FilterComponent{};
        component.uid = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component += EFilterComponentFlag::IsEquals;
        filter_tabs.components.emplace_back(std::move(component));
    }
    {
        auto component = FilterComponent{};
        component.uid = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component += EFilterComponentFlag::IsEquals;
        component += EFilterComponentFlag::IsIgnoreCase;
        component += EFilterComponentFlag::IsRegex;
        filter_tabs.components.emplace_back(std::move(component));
    }

    {
        auto filter = Filter{};
        filter.uid = UniqueID::Generate();
        filter.name = "Filter1";
        filter.component_uids.push_back(filter_tabs.components[0].uid);
        filter.component_uids.push_back(filter_tabs.components[1].uid);
        filter.colors = FilterColors{
            .foreground = ImVec4{1.0f, 1.0f, 1.0f, 1.0f},
            .background = ImVec4{0.0f, 0.0f, 0.0f, 0.0f}};
        filter.priority = 0;
        filter += EFilterFlag::IsActive;
        filter_tabs.filters.emplace_back(std::move(filter));
    }

    {
        auto tab = FilterTab{};
        tab.uid = UniqueID::Generate();
        tab.name = "Tab1";
        tab.filter_ids.push_back(filter_tabs.filters[0].uid);
        filter_tabs.tabs.emplace_back(std::move(tab));
    }

    return app_state;
}

int main()
{
    LOG_SCOPE("");

    Graphite::Core::Application::WindowConfiguration window_configuration{};
    window_configuration.width = 800;
    window_configuration.height = 570;
    window_configuration.title = "Players Manager";

    auto app =
        Graphite::Core::Application::TGraphiteApplication<Fluxion::Application::AppState>::CreateApplication<
            Fluxion::Application::FluxionApplication>(
            std::move(window_configuration), MakeDefaultAppState());

    app->Run();

    return EXIT_SUCCESS;
}
