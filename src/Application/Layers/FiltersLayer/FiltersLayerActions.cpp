/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief Main layer responsible for rendering logs table.
///

#include "FiltersLayerActions.hpp"
#include "Graphite/Logger.hpp"

namespace Fluxion::Application::Layers::Actions::FiltersLayer {

using namespace Fluxion::API::Data;

template <typename T>
auto FindByID(std::vector<std::shared_ptr<T>>& vec, Graphite::Common::UniqueID const& id)
{
    return std::find_if(vec.begin(), vec.end(), [&](const auto& ptr) { return ptr->id == id; });
}

template <EFilterActionType ActionType>
void handle(AppState& application_state, FilterActionPayload const& action) = delete;

template <>
void handle<EFilterActionType::AddFiltersTab>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(action.tab_id == std::nullopt, "tab should be nullopt for AddFiltersTab action");
    GRAPHITE_ASSERT(
        action.filter_id == std::nullopt, "filter should be nullopt for AddFiltersTab action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt, "component should be nullopt for AddFiltersTab action");

    auto& tabs{application_state.filters.tabs};
    auto& new_tab = *tabs.back.emplace_back(std::make_shared<FiltersTab>());
    new_tab.id = Graphite::Common::UniqueID::Generate();
    new_tab.name = "New Tab";
    new_tab[EFiltersTabFlag::IsActive] = true;
    new_tab.UpdateImGuiID();

    auto& new_filters{new_tab.filters};
    auto& new_filter = *new_filters.back.emplace_back(std::make_shared<Filter>());
    new_filters.MarkDataHasChanges();
    new_filter.id = Graphite::Common::UniqueID::Generate();
    new_filter.name = "New Filter";
    new_filter.colors = {
        .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
    new_filter[EFilterFlag::IsActive] = true;

    auto& new_components{new_filter.components};
    auto& new_component = *new_components.back.emplace_back(std::make_shared<FilterComponent>());
    new_filter.components.MarkDataHasChanges();
    new_component.id = Graphite::Common::UniqueID::Generate();
    new_component[EFilterComponentFlag::IsEquals] = true;
}

template <>
void handle<EFilterActionType::RemoveFiltersTab>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveFiltersTab action");
    GRAPHITE_ASSERT(
        action.filter_id == std::nullopt, "filter should be nullopt for RemoveFiltersTab action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt,
        "component should be nullopt for RemoveFiltersTab action");

    auto& tabs{application_state.filters.tabs};
    auto const tab_it = FindByID(tabs.back, *action.tab_id);
    if (tab_it != tabs.back.cend())
    {
        tabs.back.erase(tab_it);
    }
    else
    {
        LOG_WARN(
            "Failed to RemoveFiltersTab with tab ID {} because it does not exist in the data",
            *action.tab_id);
    }
}

template <>
void handle<EFilterActionType::DuplicateFiltersTab>(
    AppState& application_state,
    FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for DuplicateFiltersTab action");
    GRAPHITE_ASSERT(
        action.filter_id == std::nullopt, "filter should be nullopt for DuplicateFiltersTab action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt,
        "component should be nullopt for DuplicateFiltersTab action");

    auto& tabs{application_state.filters.tabs};
    auto const tab_it = FindByID(tabs.back, *action.tab_id);

    if (tab_it != tabs.back.cend())
    {
        auto duplicated_tab = std::make_shared<FiltersTab>(**tab_it);
        duplicated_tab->filters.MarkDataHasChanges();
        duplicated_tab->id = Graphite::Common::UniqueID::Generate();
        duplicated_tab->name += "*";
        duplicated_tab->UpdateImGuiID();
        for (auto& filter : duplicated_tab->filters.back)
        {
            filter = std::make_shared<Fluxion::API::Data::Filter>(*filter);
            filter->id = Graphite::Common::UniqueID::Generate();
            filter->components.MarkDataHasChanges();
            for (auto& component : filter->components.back)
            {
                component = std::make_shared<Fluxion::API::Data::FilterComponent>(*component);
                component->id = Graphite::Common::UniqueID::Generate();
            }
        }
        tabs.back.insert(std::next(tab_it), std::move(duplicated_tab));
    }
    else
    {
        LOG_WARN(
            "Failed to DuplicateFiltersTab with tab ID {} because it does not exist in the "
            "data",
            *action.tab_id);
    }
}

template <>
void handle<EFilterActionType::AddFilter>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(action.tab_id != std::nullopt, "tab should NOT be nullopt for AddFilter action");
    GRAPHITE_ASSERT(
        action.filter_id == std::nullopt, "filter should be nullopt for AddFilter action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt, "component should be nullopt for AddFilter action");

    auto& tabs{application_state.filters.tabs};
    auto const tab_it = FindByID(tabs.back, *action.tab_id);
    GRAPHITE_ASSERT(
        tab_it != tabs.back.end(),
        std::string{"Failed to find tab with ID "} + action.tab_id->ToString());
    auto& tab{**tab_it};

    auto& filters{tab.filters};
    filters.MarkDataHasChanges();
    auto& new_filter = *filters.back.emplace_back(std::make_shared<Filter>());
    new_filter.components.MarkDataHasChanges();
    new_filter.id = Graphite::Common::UniqueID::Generate();
    new_filter.name = "New Filter";
    new_filter.colors = {
        .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
    new_filter[EFilterFlag::IsActive] = true;

    auto& new_component =
        *new_filter.components.back.emplace_back(std::make_shared<FilterComponent>());
    new_component.id = Graphite::Common::UniqueID::Generate();
    new_component[EFilterComponentFlag::IsEquals] = true;
}

template <>
void handle<EFilterActionType::RemoveFilter>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveFilter action");
    GRAPHITE_ASSERT(
        action.filter_id != std::nullopt, "filter should NOT be nullopt for RemoveFilter action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt, "component should be nullopt for RemoveFilter action");

    auto& tabs{application_state.filters.tabs};
    auto const tab_it = FindByID(tabs.back, *action.tab_id);
    GRAPHITE_ASSERT(
        tab_it != tabs.back.end(),
        std::string{"Failed to find tab with ID "} + action.tab_id->ToString());
    auto& tab{**tab_it};

    auto& filters = tab.filters;
    filters.MarkDataHasChanges();
    auto const filter_it = FindByID(filters.back, *action.filter_id);

    if (filter_it != filters.back.cend())
    {
        filters.back.erase(filter_it);
    }
    else
    {
        LOG_WARN(
            "Failed to RemoveFilter with filter ID {} because it does not exist in the data",
            *action.filter_id);
    }
}

template <>
void handle<EFilterActionType::DuplicateFilter>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for DuplicateFilter action");
    GRAPHITE_ASSERT(
        action.filter_id != std::nullopt, "filter should NOT be nullopt for DuplicateFilter action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt,
        "component should be nullopt for DuplicateFilter action");

    auto& tabs{application_state.filters.tabs};
    auto const tab_it = FindByID(tabs.back, *action.tab_id);
    GRAPHITE_ASSERT(
        tab_it != tabs.back.end(),
        std::string{"Failed to find tab with ID "} + action.tab_id->ToString());
    auto& tab{**tab_it};

    auto& filters = tab.filters;
    filters.MarkDataHasChanges();
    auto const filter_it = FindByID(filters.back, *action.filter_id);

    if (filter_it != filters.back.cend())
    {
        auto duplicate_filter = std::make_shared<Filter>(**filter_it);
        duplicate_filter->components.MarkDataHasChanges();
        duplicate_filter->id = Graphite::Common::UniqueID::Generate();
        duplicate_filter->name += "*";
        for (auto& component : duplicate_filter->components.back)
        {
            component = std::make_shared<Fluxion::API::Data::FilterComponent>(*component);
            component->id = Graphite::Common::UniqueID::Generate();
        }
        filters.back.insert(std::next(filter_it), std::move(duplicate_filter));
    }
    else
    {
        LOG_WARN(
            "Failed to DuplicateFilter with filter ID {} because it does not exist in the data",
            *action.filter_id);
    }
}

template <>
void handle<EFilterActionType::AddFilterComponent>(
    AppState& application_state,
    FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for AddFilterComponent action");
    GRAPHITE_ASSERT(
        action.filter_id != std::nullopt,
        "filter should NOT be nullopt for AddFilterComponent action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt,
        "component should NOT be nullopt for AddFilterComponent action");

    auto& tabs{application_state.filters.tabs};
    auto const tab_it = FindByID(tabs.back, *action.tab_id);
    GRAPHITE_ASSERT(
        tab_it != tabs.back.end(),
        std::string{"Failed to find tab with ID "} + action.tab_id->ToString());
    auto& tab{**tab_it};

    auto& filters{tab.filters};
    filters.MarkDataHasChanges();
    auto const filter_it = FindByID(filters.back, *action.filter_id);
    GRAPHITE_ASSERT(
        filter_it != filters.back.end(),
        std::string{"Failed to find filter with ID "} + action.tab_id->ToString());
    auto& filter{**filter_it};
    filter.components.MarkDataHasChanges();

    auto& new_component = *filter.components.back.emplace_back(std::make_shared<FilterComponent>());
    new_component.id = Graphite::Common::UniqueID::Generate();
    new_component[EFilterComponentFlag::IsEquals] = true;
}

template <>
void handle<EFilterActionType::RemoveFilterComponent>(
    AppState& application_state,
    FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveFilterComponent action");
    GRAPHITE_ASSERT(
        action.filter_id != std::nullopt,
        "filter should NOT be nullopt for RemoveFilterComponent action");
    GRAPHITE_ASSERT(
        action.component_id != std::nullopt,
        "component should NOT be nullopt for RemoveFilterComponent action");

    auto& tabs{application_state.filters.tabs};
    auto const tab_it = FindByID(tabs.back, *action.tab_id);
    GRAPHITE_ASSERT(
        tab_it != tabs.back.end(),
        std::string{"Failed to find tab with ID "} + action.tab_id->ToString());
    auto& tab{**tab_it};

    auto& filters{tab.filters};
    auto const filter_it = FindByID(filters.back, *action.filter_id);
    GRAPHITE_ASSERT(
        filter_it != filters.back.end(),
        std::string{"Failed to find filter with ID "} + action.tab_id->ToString());
    filters.MarkDataHasChanges();

    auto& components{(**filter_it).components};
    components.MarkDataHasChanges();
    auto const component_it = FindByID(components.back, *action.component_id);

    if (component_it != components.back.end())
    {
        components.back.erase(component_it);
    }
    else
    {
        LOG_WARN(
            "Failed to RemoveFilterComponent with component ID {} because it does not exist in "
            "the data",
            *action.component_id);
    }
}

void HandleFiltersLayerAction(AppState& application_state, FilterActionPayload const& action)
{
    if (action.type == EFilterActionType::None)
    {
        return;
    }

    LOG_TRACE(
        "Handling action type {} -> tab: {} | filter: {} | component: {}",
        static_cast<std::uint32_t>(action.type),
        action.tab_id ? action.tab_id->ToString() : "nullopt",
        action.filter_id ? action.filter_id->ToString() : "nullopt",
        action.component_id ? action.component_id->ToString() : "nullopt");

    switch (action.type)
    {
    case EFilterActionType::AddFiltersTab: {
        handle<EFilterActionType::AddFiltersTab>(application_state, action);
        break;
    }
    case EFilterActionType::RemoveFiltersTab: {
        handle<EFilterActionType::RemoveFiltersTab>(application_state, action);
        break;
    }
    case EFilterActionType::DuplicateFiltersTab: {
        handle<EFilterActionType::DuplicateFiltersTab>(application_state, action);
        break;
    }
    case EFilterActionType::AddFilter: {
        handle<EFilterActionType::AddFilter>(application_state, action);
        break;
    }
    case EFilterActionType::RemoveFilter: {
        handle<EFilterActionType::RemoveFilter>(application_state, action);
        break;
    }
    case EFilterActionType::DuplicateFilter: {
        handle<EFilterActionType::DuplicateFilter>(application_state, action);
        break;
    }
    case EFilterActionType::AddFilterComponent: {
        handle<EFilterActionType::AddFilterComponent>(application_state, action);
        break;
    }
    case EFilterActionType::RemoveFilterComponent: {
        handle<EFilterActionType::RemoveFilterComponent>(application_state, action);
        break;
    }
    default: {
        LOG_WARN(
            "Unknown action type {} -> tab: {} | filter: {} | component: {}",
            static_cast<std::uint32_t>(action.type),
            action.tab_id ? action.tab_id->ToString() : "nullopt",
            action.filter_id ? action.filter_id->ToString() : "nullopt",
            action.component_id ? action.component_id->ToString() : "nullopt");
        break;
    }
    }

    if (application_state.filters.tabs.back.empty())
    {
        application_state.filters.tabs.back = DefaultState::MakeDefaultTabs();
    }

    application_state.filters.tabs.MarkDataHasChanges();
}

} // namespace Fluxion::Application::Layers::Actions::FiltersLayer
