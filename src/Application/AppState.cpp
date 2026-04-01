/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Implementation of @see AppState.hpp.
///

#include "AppState.hpp"

namespace Fluxion::Application::DefaultState {
AppState Make()
{
    AppState app_state{};

    auto default_tabs = MakeDefaultTabs();

    // app_state.filters.tabs is a TCopyDoubleBuffer
    // We initialize both front and back with the same starting vector.
    // Since it's a vector of shared_ptr, it's just copying pointers, not the Tab objects.
    app_state.filters.tabs.Init(std::vector(default_tabs), std::move(default_tabs));

    return app_state;
}

std::vector<Fluxion::API::Data::FiltersTab::Ptr> MakeDefaultTabs()
{
    using namespace Fluxion::API::Data;
    using UniqueID = Graphite::Common::Utility::UniqueID;

    // Create the primary tab
    auto tab_ptr = std::make_shared<FiltersTab>();
    tab_ptr->id = UniqueID::Generate();
    tab_ptr->name = "Tab1";
    (*tab_ptr)[EFiltersTabFlag::IsActive] = true;
    tab_ptr->UpdateImGuiID();

    // Create the filter
    auto filter_ptr = std::make_shared<Filter>();
    filter_ptr->id = UniqueID::Generate();
    filter_ptr->name = "Filter1";
    filter_ptr->colors =
        FilterColors{.foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.2f}};
    (*filter_ptr)[EFilterFlag::IsActive] = true;

    // Create the component
    auto component_ptr = std::make_shared<FilterComponent>();
    component_ptr->id = UniqueID::Generate();
    component_ptr->over_field_id = UniqueID::Default();
    (*component_ptr)[EFilterComponentFlag::IsEquals] = true;

    // 1. Initialize Component Double Buffer
    // We provide two vectors containing the same shared_ptr
    filter_ptr->components.Init({component_ptr}, {component_ptr});

    // 2. Initialize Filter Double Buffer
    tab_ptr->filters.Init({filter_ptr}, {filter_ptr});

    // Return as a single vector (this will be used by AppState::Make to Init)
    return {tab_ptr};
}

} // namespace Fluxion::Application::DefaultState
