/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include <memory>
#include <vector>
#include "AppState.hpp"
#include "Graphite/Logger.hpp"

namespace Fluxion::Application::Layers::Actions {

enum class EFilterActionType : std::uint8_t
{
    None = 0,
    AddFiltersTab,
    RemoveFiltersTab,
    DuplicateFiltersTab,
    AddFilter,
    RemoveFilter,
    DuplicateFilter,
    AddFilterComponent,
    RemoveFilterComponent
};

struct FilterActionPayload
{
    EFilterActionType type{EFilterActionType::None};
    std::optional<Graphite::Common::UniqueID> tab_id{};
    std::optional<Graphite::Common::UniqueID> filter_id{};
    std::optional<Graphite::Common::UniqueID> component_id{};
};

inline void HandleFiltersLayerAction(AppState& application_state, FilterActionPayload const& action)
{
    if (action.type == EFilterActionType::None)
    {
        return;
    }

    LOG_TRACE(
        "Handling action type {} -> tab: {} | filter: {} | component: {}",
        static_cast<std::uint32_t>(action.type),
        action.tab_id ? action.tab_id->toString() : "nullptr",
        action.filter_id ? action.filter_id->toString() : "nullptr",
        action.component_id ? action.component_id->toString() : "nullptr");

    application_state.filters.dirty = true;

    using namespace Fluxion::API::Data;

    auto& tabs{application_state.filters.tabs};
    switch (action.type)
    {
    case EFilterActionType::AddFiltersTab: {
        GRAPHITE_ASSERT(
            action.tab_id == std::nullopt, "tab should be nullopt for AddFiltersTab action");
        GRAPHITE_ASSERT(
            action.filter_id == std::nullopt, "filter should be nullopt for AddFiltersTab action");
        GRAPHITE_ASSERT(
            action.component_id == std::nullopt,
            "component should be nullopt for AddFiltersTab action");

        auto& new_tab = *tabs.back.emplace_back(std::make_shared<FiltersTab>());
        new_tab.id = Graphite::Common::UniqueID::Generate();
        new_tab.name = "New Tab";
        new_tab[EFiltersTabFlag::IsActive] = true;

        auto& new_filter = *new_tab.filters.back.emplace_back(std::make_shared<Filter>());
        new_tab.filters.MarkDataHasChanges();

        new_filter.id = Graphite::Common::UniqueID::Generate();
        new_filter.name = "New Filter";
        new_filter.colors = {
            .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
        new_filter[EFilterFlag::IsActive] = true;

        auto& new_component =
            *new_filter.components.back.emplace_back(std::make_shared<FilterComponent>());
        new_filter.components.MarkDataHasChanges();
        new_component.id = Graphite::Common::UniqueID::Generate();
        new_component[EFilterComponentFlag::IsEquals] = true;

        break;
    }
    case EFilterActionType::RemoveFiltersTab: {
        GRAPHITE_ASSERT(
            action.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveFiltersTab action");
        GRAPHITE_ASSERT(
            action.filter_id == std::nullopt,
            "filter should be nullopt for RemoveFiltersTab action");
        GRAPHITE_ASSERT(
            action.component_id == std::nullopt,
            "component should be nullopt for RemoveFiltersTab action");

        auto const it = std::find_if(
            tabs.back.cbegin(), tabs.back.cend(), [&target_id = action.tab_id](auto const& tab_ptr) {
                return tab_ptr->id == target_id;
            });

        if (it != tabs.back.cend())
        {
            tabs.back.erase(it);
        }
        else
        {
            LOG_WARN(
                "Failed to RemoveFiltersTab with tab ID {} because it does not exist in the data",
                *action.tab_id);
        }
        break;
    }
    case EFilterActionType::DuplicateFiltersTab: {
        GRAPHITE_ASSERT(
            action.tab_id != std::nullopt,
            "tab should NOT be nullopt for DuplicateFiltersTab action");
        GRAPHITE_ASSERT(
            action.filter_id == std::nullopt,
            "filter should be nullopt for DuplicateFiltersTab action");
        GRAPHITE_ASSERT(
            action.component_id == std::nullopt,
            "component should be nullopt for DuplicateFiltersTab action");

        auto const it = std::find_if(
            tabs.back.cbegin(), tabs.back.cend(), [&target_id = action.tab_id](auto const& tab_ptr) {
                return tab_ptr->id == target_id;
            });

        if (it != tabs.back.cend())
        {
            auto duplicate_tab = std::make_shared<FiltersTab>(**it);
            duplicate_tab->filters.MarkDataHasChanges();
            duplicate_tab->id = Graphite::Common::UniqueID::Generate();
            duplicate_tab->name += "*";
            for (auto& filter : duplicate_tab->filters.back)
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
            tabs.back.insert(std::next(it), std::move(duplicate_tab));
        }
        else
        {
            LOG_WARN(
                "Failed to DuplicateFiltersTab with tab ID {} because it does not exist in the "
                "data",
                *action.tab_id);
        }
        break;
    }
    case EFilterActionType::AddFilter: {
        GRAPHITE_ASSERT(
            action.tab_id != std::nullopt, "tab should NOT be nullopt for AddFilter action");
        GRAPHITE_ASSERT(
            action.filter_id == std::nullopt, "filter should be nullopt for AddFilter action");
        GRAPHITE_ASSERT(
            action.component_id == std::nullopt, "component should be nullopt for AddFilter action");

        auto tab_it = std::find_if(
            application_state.filters.tabs.back.begin(),
            application_state.filters.tabs.back.end(),
            [&target_id = *action.tab_id](auto const& tab) { return tab->id == target_id; });
        GRAPHITE_ASSERT(
            tab_it != application_state.filters.tabs.back.end(),
            std::string{"Failed to find tab with ID "} + action.tab_id->toString());
        (**tab_it).filters.MarkDataHasChanges();

        auto& filters{(**tab_it).filters};
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

        break;
    }
    case EFilterActionType::RemoveFilter: {
        GRAPHITE_ASSERT(
            action.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveFilter action");
        GRAPHITE_ASSERT(
            action.filter_id != std::nullopt,
            "filter should NOT be nullopt for RemoveFilter action");
        GRAPHITE_ASSERT(
            action.component_id == std::nullopt,
            "component should be nullopt for RemoveFilter action");

        auto tab_it = std::find_if(
            application_state.filters.tabs.back.begin(),
            application_state.filters.tabs.back.end(),
            [&target_id = *action.tab_id](auto const& tab) { return tab->id == target_id; });
        GRAPHITE_ASSERT(
            tab_it != application_state.filters.tabs.back.end(),
            std::string{"Failed to find tab with ID "} + action.tab_id->toString());
        (**tab_it).filters.MarkDataHasChanges();

        auto& filters = (**tab_it).filters;
        auto const it = std::find_if(
            filters.back.cbegin(),
            filters.back.cend(),
            [&target_id = action.filter_id](auto const& filter) { return filter->id == target_id; });

        if (it != filters.back.cend())
        {
            filters.back.erase(it);
        }
        else
        {
            LOG_WARN(
                "Failed to RemoveFilter with filter ID {} because it does not exist in the data",
                *action.filter_id);
        }
        break;
    }
    case EFilterActionType::DuplicateFilter: {
        GRAPHITE_ASSERT(
            action.tab_id != std::nullopt, "tab should NOT be nullopt for DuplicateFilter action");
        GRAPHITE_ASSERT(
            action.filter_id != std::nullopt,
            "filter should NOT be nullopt for DuplicateFilter action");
        GRAPHITE_ASSERT(
            action.component_id == std::nullopt,
            "component should be nullopt for DuplicateFilter action");

        auto tab_it = std::find_if(
            application_state.filters.tabs.back.begin(),
            application_state.filters.tabs.back.end(),
            [&target_id = *action.tab_id](auto const& tab) { return tab->id == target_id; });
        GRAPHITE_ASSERT(
            tab_it != application_state.filters.tabs.back.end(),
            std::string{"Failed to find tab with ID "} + action.tab_id->toString());
        (**tab_it).filters.MarkDataHasChanges();

        auto& filters = (**tab_it).filters;
        auto const it = std::find_if(
            filters.back.cbegin(),
            filters.back.cend(),
            [&target_id = action.filter_id](auto const& filter) { return filter->id == target_id; });

        if (it != filters.back.cend())
        {
            auto duplicate_filter = std::make_shared<Filter>(**it);
            duplicate_filter->components.MarkDataHasChanges();
            duplicate_filter->id = Graphite::Common::UniqueID::Generate();
            duplicate_filter->name += "*";
            for (auto& component : duplicate_filter->components.back)
            {
                component = std::make_shared<Fluxion::API::Data::FilterComponent>(*component);
                component->id = Graphite::Common::UniqueID::Generate();
            }
            filters.back.insert(std::next(it), std::move(duplicate_filter));
        }
        else
        {
            LOG_WARN(
                "Failed to DuplicateFilter with filter ID {} because it does not exist in the data",
                *action.filter_id);
        }
        break;
    }
    case EFilterActionType::AddFilterComponent: {
        GRAPHITE_ASSERT(
            action.tab_id != std::nullopt,
            "tab should NOT be nullopt for AddFilterComponent action");
        GRAPHITE_ASSERT(
            action.filter_id != std::nullopt,
            "filter should NOT be nullopt for AddFilterComponent action");
        GRAPHITE_ASSERT(
            action.component_id == std::nullopt,
            "component should NOT be nullopt for AddFilterComponent action");

        auto tab_it = std::find_if(
            application_state.filters.tabs.back.begin(),
            application_state.filters.tabs.back.end(),
            [&target_id = *action.tab_id](auto const& tab) { return tab->id == target_id; });
        GRAPHITE_ASSERT(
            tab_it != application_state.filters.tabs.back.end(),
            std::string{"Failed to find tab with ID "} + action.tab_id->toString());
        (**tab_it).filters.MarkDataHasChanges();

        auto filter_it = std::find_if(
            (**tab_it).filters.back.begin(),
            (**tab_it).filters.back.end(),
            [&target_id = *action.filter_id](auto const& filter) { return filter->id == target_id; });
        GRAPHITE_ASSERT(
            filter_it != (**tab_it).filters.back.end(),
            std::string{"Failed to find filter with ID "} + action.tab_id->toString());
        (**filter_it).components.MarkDataHasChanges();

        auto& new_component =
            *(**filter_it).components.back.emplace_back(std::make_shared<FilterComponent>());
        new_component.id = Graphite::Common::UniqueID::Generate();
        new_component[EFilterComponentFlag::IsEquals] = true;

        break;
    }
    case EFilterActionType::RemoveFilterComponent: {
        GRAPHITE_ASSERT(
            action.tab_id != std::nullopt,
            "tab should NOT be nullopt for RemoveFilterComponent action");
        GRAPHITE_ASSERT(
            action.filter_id != std::nullopt,
            "filter should NOT be nullopt for RemoveFilterComponent action");
        GRAPHITE_ASSERT(
            action.component_id != std::nullopt,
            "component should NOT be nullopt for RemoveFilterComponent action");

        auto tab_it = std::find_if(
            application_state.filters.tabs.back.begin(),
            application_state.filters.tabs.back.end(),
            [&target_id = *action.tab_id](auto const& tab) { return tab->id == target_id; });
        GRAPHITE_ASSERT(
            tab_it != application_state.filters.tabs.back.end(),
            std::string{"Failed to find tab with ID "} + action.tab_id->toString());

        auto filter_it = std::find_if(
            (**tab_it).filters.back.begin(),
            (**tab_it).filters.back.end(),
            [&target_id = *action.filter_id](auto const& filter) { return filter->id == target_id; });
        GRAPHITE_ASSERT(
            filter_it != (**tab_it).filters.back.end(),
            std::string{"Failed to find filter with ID "} + action.tab_id->toString());
        (**tab_it).filters.MarkDataHasChanges();

        (**filter_it).components.MarkDataHasChanges();
        auto& components = (**filter_it).components.back;
        auto const it = std::find_if(
            components.cbegin(),
            components.cend(),
            [&target_id = action.component_id](auto const& component) {
                return component->id == target_id;
            });

        if (it != components.cend())
        {
            components.erase(it);
            application_state.filters.dirty = true;
        }
        else
        {
            LOG_WARN(
                "Failed to RemoveFilterComponent with component ID {} because it does not exist in "
                "the data",
                *action.component_id);
        }
        break;
    }
    default: {
        LOG_WARN(
            "Unknown action type {} -> tab: {} | filter: {} | component: {}",
            static_cast<std::uint32_t>(action.type),
            action.tab_id ? action.tab_id->toString() : "nullopt",
            action.filter_id ? action.filter_id->toString() : "nullopt",
            action.component_id ? action.component_id->toString() : "nullopt");
        break;
    }
    }

    if (application_state.filters.tabs.back.empty())
    {
        application_state.filters.tabs.back = DefaultState::MakeDefaultTabs();
    }

    application_state.filters.tabs.MarkDataHasChanges();
}

} // namespace Fluxion::Application::Layers::Actions
