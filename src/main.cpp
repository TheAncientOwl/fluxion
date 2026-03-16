/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file main.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
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

    // Tab 1
    {
        auto component = FilterComponent{};
        component.id = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component[EFilterComponentFlag::IsEquals] = true;
        filter_tabs.components.emplace_back(std::move(component));
    }
    {
        auto component = FilterComponent{};
        component.id = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component[EFilterComponentFlag::IsEquals] = true;
        component[EFilterComponentFlag::IsCaseSensitive] = true;
        component[EFilterComponentFlag::IsRegex] = true;
        filter_tabs.components.emplace_back(std::move(component));
    }
    {
        auto component = FilterComponent{};
        component.id = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component[EFilterComponentFlag::IsEquals] = true;
        filter_tabs.components.emplace_back(std::move(component));
    }
    {
        auto component = FilterComponent{};
        component.id = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component[EFilterComponentFlag::IsEquals] = true;
        component[EFilterComponentFlag::IsCaseSensitive] = true;
        component[EFilterComponentFlag::IsRegex] = true;
        filter_tabs.components.emplace_back(std::move(component));
    }

    {
        auto filter = Filter{};
        filter.id = UniqueID::Generate();
        filter.name = "> Filter 11";
        filter.component_ids.push_back(filter_tabs.components[0].id);
        filter.component_ids.push_back(filter_tabs.components[1].id);
        filter.colors = FilterColors{
            .foreground = ImVec4{1.0f, 1.0f, 1.0f, 1.0f},
            .background = ImVec4{0.0f, 0.0f, 0.0f, 0.2f}};
        filter.priority = 0;
        filter[EFilterFlag::IsActive] = true;
        filter_tabs.filters.emplace_back(std::move(filter));
    }
    {
        auto filter = Filter{};
        filter.id = UniqueID::Generate();
        filter.name = "> Filter 12";
        filter.component_ids.push_back(filter_tabs.components[2].id);
        filter.component_ids.push_back(filter_tabs.components[3].id);
        filter.colors = FilterColors{
            .foreground = ImVec4{1.0f, 1.0f, 1.0f, 1.0f},
            .background = ImVec4{0.0f, 0.0f, 0.0f, 0.2f}};
        filter.priority = 0;
        filter[EFilterFlag::IsActive] = true;
        filter_tabs.filters.emplace_back(std::move(filter));
    }

    {
        auto tab = FilterTab{};
        tab.id = UniqueID::Generate();
        tab.name = "> Tab 1";
        tab.filter_ids.push_back(filter_tabs.filters[0].id);
        tab.filter_ids.push_back(filter_tabs.filters[1].id);
        filter_tabs.tabs.emplace_back(std::move(tab));
    }

    // Tab 2
    {
        auto component = FilterComponent{};
        component.id = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component[EFilterComponentFlag::IsEquals] = true;
        filter_tabs.components.emplace_back(std::move(component));
    }
    {
        auto component = FilterComponent{};
        component.id = UniqueID::Generate();
        component.over_field_id =
            UniqueID::Default(); // TODO: this should come from plugins, so no filter can be created
        component[EFilterComponentFlag::IsEquals] = true;
        component[EFilterComponentFlag::IsCaseSensitive] = true;
        component[EFilterComponentFlag::IsRegex] = true;
        filter_tabs.components.emplace_back(std::move(component));
    }

    {
        auto filter = Filter{};
        filter.id = UniqueID::Generate();
        filter.name = "> Filter 21";
        filter.component_ids.push_back(filter_tabs.components[4].id);
        filter.component_ids.push_back(filter_tabs.components[5].id);
        filter.colors = FilterColors{
            .foreground = ImVec4{1.0f, 1.0f, 1.0f, 1.0f},
            .background = ImVec4{0.0f, 0.0f, 0.0f, 0.2f}};
        filter.priority = 0;
        filter[EFilterFlag::IsActive] = true;
        filter_tabs.filters.emplace_back(std::move(filter));
    }

    {
        auto tab = FilterTab{};
        tab.id = UniqueID::Generate();
        tab.name = "> Tab 2";
        tab.filter_ids.push_back(filter_tabs.filters[2].id);
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
    window_configuration.title = "Fluxion";

    auto app =
        Graphite::Core::Application::TGraphiteApplication<Fluxion::Application::AppState>::CreateApplication<
            Fluxion::Application::FluxionApplication>(
            std::move(window_configuration), MakeDefaultAppState());

    app->Run();

    return EXIT_SUCCESS;
}
