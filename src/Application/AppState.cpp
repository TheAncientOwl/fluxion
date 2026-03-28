/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Implementation of @see AppState.hpp.
///

#include "AppState.hpp"

namespace Fluxion::Application::DefaultState {

AppState Make()
{
    AppState app_state{};

    app_state.filters.tabs.back = MakeDefaultTabs();
    app_state.filters.tabs.front = app_state.filters.tabs.back;

    return app_state;
}

Fluxion::API::Data::FiltersTabs::StorageType MakeDefaultTabs()
{
    using namespace Fluxion::API::Data;
    using UniqueID = Graphite::Common::UniqueID;

    FiltersTabs::StorageType tabs{};
    {
        auto tab_ptr = tabs.emplace_back(std::make_shared<FiltersTab>());
        auto& tab = *tab_ptr;
        tab.id = UniqueID::Generate();
        tab.name = "Tab1";
        tab[EFiltersTabFlag::IsActive] = true;

        {
            auto filter_ptr = tab.filters.back.emplace_back(std::make_shared<Filter>());
            tab.filters.front.emplace_back(filter_ptr);

            auto& filter = *filter_ptr;
            filter.id = UniqueID::Generate();
            filter.name = "Filter1";
            filter.colors = FilterColors{
                .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.2f}};
            filter[EFilterFlag::IsActive] = true;
            filter[EFilterFlag::IsHighlightOnly] = false;
            filter[EFilterFlag::IsCollapsed] = false;

            {
                auto component_ptr =
                    filter.components.back.emplace_back(std::make_shared<FilterComponent>());
                auto& component = *component_ptr;
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
