/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file main.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief ImGui entry point.
///

#include <stdlib.h>

#include "Application/Fluxion.hpp"
#include "Graphite/Logger/Logger.hpp"

Fluxion::Application::AppState MakeDefaultAppState()
{
    Fluxion::Application::AppState app_state{};

    using namespace Fluxion::API::Data;
    using UniqueID = Graphite::Common::UniqueID;

    auto& tabs{app_state.filters.tabs};
    {
        auto& tab = *tabs.emplace_back(std::make_shared<FiltersTab>());
        tab.id = UniqueID::Generate();
        tab.name = "Tab1";
        tab[EFiltersTabFlag::IsActive] = true;

        {
            auto& filter = *tab.filters.emplace_back(std::make_shared<Filter>());
            filter.id = UniqueID::Generate();
            filter.name = "Filter1 @1";
            filter.colors = FilterColors{
                .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.2f}};
            filter[EFilterFlag::IsActive] = true;
            filter[EFilterFlag::IsHighlightOnly] = false;
            filter[EFilterFlag::IsCollapsed] = false;

            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = true;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = false;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = false;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
        }

        {
            auto& filter = *tab.filters.emplace_back(std::make_shared<Filter>());
            filter.id = UniqueID::Generate();
            filter.name = "Filter2 @1";
            filter.colors = FilterColors{
                .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.2f}};
            filter[EFilterFlag::IsActive] = true;
            filter[EFilterFlag::IsHighlightOnly] = true;
            filter[EFilterFlag::IsCollapsed] = false;

            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = true;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = false;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = false;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
        }
    }
    {
        auto& tab = *tabs.emplace_back(std::make_shared<FiltersTab>());
        tab.id = UniqueID::Generate();
        tab.name = "Tab2";
        tab[EFiltersTabFlag::IsActive] = true;

        {
            auto& filter = *tab.filters.emplace_back(std::make_shared<Filter>());
            filter.id = UniqueID::Generate();
            filter.name = "Filter1 @2";
            filter.colors = FilterColors{
                .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.2f}};
            filter[EFilterFlag::IsActive] = true;
            filter[EFilterFlag::IsHighlightOnly] = false;
            filter[EFilterFlag::IsCollapsed] = false;

            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = true;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = false;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = false;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
        }

        {
            auto& filter = *tab.filters.emplace_back(std::make_shared<Filter>());
            filter.id = UniqueID::Generate();
            filter.name = "Filter2 @2";
            filter.colors = FilterColors{
                .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.2f}};
            filter[EFilterFlag::IsActive] = true;
            filter[EFilterFlag::IsHighlightOnly] = true;
            filter[EFilterFlag::IsCollapsed] = false;

            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = true;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = false;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = false;
            }
            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = true;
                component[EFilterComponentFlag::IsEquals] = false;
                component[EFilterComponentFlag::IsCaseSensitive] = true;
            }
        }
    }
    return app_state;
}

int main()
{
    Graphite::Logger::Logger::LoadConfig();

    LOG_SCOPE("");

    Graphite::Application::WindowConfiguration window_configuration{};
    window_configuration.width = 950;
    window_configuration.height = 750;
    window_configuration.title = "Fluxion";

    auto app =
        Graphite::Application::TGraphiteApplication<Fluxion::Application::AppState>::CreateApplication<
            Fluxion::Application::FluxionApplication>(
            std::move(window_configuration), MakeDefaultAppState());

    app->Run();

    Graphite::Logger::Logger::SaveConfig();

    return EXIT_SUCCESS;
}
