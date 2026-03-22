/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see AppState.hpp.
///

#include "AppState.hpp"

namespace Fluxion::Application::DefaultState {

AppState Make()
{
    AppState app_state{};

    app_state.filters.tabs = MakeDefaultTabs();

    return app_state;
}

Fluxion::API::Data::FiltersTabs MakeDefaultTabs()
{
    Fluxion::Application::AppState app_state{};

    using namespace Fluxion::API::Data;
    using UniqueID = Graphite::Common::UniqueID;

    FiltersTabs tabs{};
    {
        auto& tab = *tabs.emplace_back(std::make_shared<FiltersTab>());
        tab.id = UniqueID::Generate();
        tab.name = "Tab1";
        tab[EFiltersTabFlag::IsActive] = true;

        {
            auto& filter = *tab.filters.emplace_back(std::make_shared<Filter>());
            filter.id = UniqueID::Generate();
            filter.name = "Filter1";
            filter.colors = FilterColors{
                .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.2f}};
            filter[EFilterFlag::IsActive] = true;
            filter[EFilterFlag::IsHighlightOnly] = false;
            filter[EFilterFlag::IsCollapsed] = false;

            {
                auto& component = *filter.components.emplace_back(std::make_shared<FilterComponent>());
                component.id = UniqueID::Generate();
                component[EFilterComponentFlag::IsRegex] = false;
                component[EFilterComponentFlag::IsEquals] = true;
                component[EFilterComponentFlag::IsCaseSensitive] = false;
            }
        }
    }
    return tabs;
}

} // namespace Fluxion::Application::DefaultState
