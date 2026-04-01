/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Main layer responsible for rendering logs table.
///

#include "FiltersLayerActions.hpp"
#include "Graphite/Logger.hpp"

namespace Fluxion::Application::Layers::Actions::FiltersLayer {

using namespace Fluxion::API::Data;

template <typename T>
auto FindByID(std::vector<std::shared_ptr<T>>& vec, Graphite::Common::Utility::UniqueID const& id)
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

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_ptr = tabs_back.emplace_back(std::make_shared<FiltersTab>());
        tab_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
        tab_ptr->name = "New Tab";
        (*tab_ptr)[EFiltersTabFlag::IsActive] = true;
        tab_ptr->UpdateImGuiID();

        // Initialize nested structures
        auto filter_ptr = std::make_shared<Filter>();
        filter_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
        filter_ptr->name = "New Filter";
        filter_ptr->colors = {
            .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
        (*filter_ptr)[EFilterFlag::IsActive] = true;

        auto comp_ptr = std::make_shared<FilterComponent>();
        comp_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
        (*comp_ptr)[EFilterComponentFlag::IsEquals] = true;

        filter_ptr->components.Init({comp_ptr}, {comp_ptr});
        tab_ptr->filters.Init({filter_ptr}, {filter_ptr});
    });
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

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto const tab_it = FindByID(tabs_back, *action.tab_id);
        if (tab_it != tabs_back.end())
        {
            tabs_back.erase(tab_it);
        }
        else
        {
            LOG_WARN(
                "Failed to RemoveFiltersTab with tab ID {} because it does not exist", *action.tab_id);
        }
    });
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

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto const tab_it = FindByID(tabs_back, *action.tab_id);
        if (tab_it == tabs_back.end())
        {
            LOG_WARN(
                "Failed to DuplicateFiltersTab with tab ID {} because it does not exist",
                *action.tab_id);
            return;
        }

        auto duplicated_tab = std::make_shared<FiltersTab>(**tab_it);
        duplicated_tab->id = Graphite::Common::Utility::UniqueID::Generate();
        duplicated_tab->name += "*";
        duplicated_tab->UpdateImGuiID();

        // Deep copy nested buffers using the public GetBack() and Init()
        auto current_filters = duplicated_tab->filters.GetBack();
        std::vector<Filter::Ptr> new_filters_list;

        for (auto& filter_ptr : current_filters)
        {
            auto filter_dup = std::make_shared<Filter>(*filter_ptr);
            filter_dup->id = Graphite::Common::Utility::UniqueID::Generate();

            auto current_comps = filter_dup->components.GetBack();
            std::vector<FilterComponent::Ptr> new_comps_list;
            for (auto& comp_ptr : current_comps)
            {
                auto comp_dup = std::make_shared<FilterComponent>(*comp_ptr);
                comp_dup->id = Graphite::Common::Utility::UniqueID::Generate();
                new_comps_list.push_back(std::move(comp_dup));
            }
            filter_dup->components.Init(std::vector(new_comps_list), std::move(new_comps_list));
            new_filters_list.push_back(std::move(filter_dup));
        }
        duplicated_tab->filters.Init(std::vector(new_filters_list), std::move(new_filters_list));

        tabs_back.insert(std::next(tab_it), std::move(duplicated_tab));
    });
}

template <>
void handle<EFilterActionType::AddFilter>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(action.tab_id != std::nullopt, "tab should NOT be nullopt for AddFilter action");
    GRAPHITE_ASSERT(
        action.filter_id == std::nullopt, "filter should be nullopt for AddFilter action");
    GRAPHITE_ASSERT(
        action.component_id == std::nullopt, "component should be nullopt for AddFilter action");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *action.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + action.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto new_filter = filters_back.emplace_back(std::make_shared<Filter>());
            new_filter->id = Graphite::Common::Utility::UniqueID::Generate();
            new_filter->name = "New Filter";
            new_filter->colors = {
                .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
            (*new_filter)[EFilterFlag::IsActive] = true;

            auto comp_ptr = std::make_shared<FilterComponent>();
            comp_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
            (*comp_ptr)[EFilterComponentFlag::IsEquals] = true;
            new_filter->components.Init({comp_ptr}, {comp_ptr});
        });
    });
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

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *action.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + action.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *action.filter_id);
            if (filter_it != filters_back.end())
            {
                filters_back.erase(filter_it);
            }
            else
            {
                LOG_WARN(
                    "Failed to RemoveFilter with filter ID {} because it does not exist",
                    *action.filter_id);
            }
        });
    });
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

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *action.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + action.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *action.filter_id);
            if (filter_it == filters_back.end())
            {
                LOG_WARN(
                    "Failed to DuplicateFilter with filter ID {} because it does not exist",
                    *action.filter_id);
                return;
            }

            auto duplicate_filter = std::make_shared<Filter>(**filter_it);
            duplicate_filter->id = Graphite::Common::Utility::UniqueID::Generate();
            duplicate_filter->name += "*";

            auto current_comps = duplicate_filter->components.GetBack();
            std::vector<FilterComponent::Ptr> new_comps_list;
            for (auto& comp_ptr : current_comps)
            {
                auto comp_dup = std::make_shared<FilterComponent>(*comp_ptr);
                comp_dup->id = Graphite::Common::Utility::UniqueID::Generate();
                new_comps_list.push_back(std::move(comp_dup));
            }
            duplicate_filter->components.Init(std::vector(new_comps_list), std::move(new_comps_list));

            filters_back.insert(std::next(filter_it), std::move(duplicate_filter));
        });
    });
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
        "component should be nullopt for AddFilterComponent action");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *action.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + action.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *action.filter_id);
            GRAPHITE_ASSERT(
                filter_it != filters_back.end(),
                "Failed to find filter with ID " + action.filter_id->ToString());

            (*filter_it)->components.UpdateBackBufferCopy([&](auto& comps_back) {
                auto new_comp = comps_back.emplace_back(std::make_shared<FilterComponent>());
                new_comp->id = Graphite::Common::Utility::UniqueID::Generate();
                (*new_comp)[EFilterComponentFlag::IsEquals] = true;
            });
        });
    });
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

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *action.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + action.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *action.filter_id);
            GRAPHITE_ASSERT(
                filter_it != filters_back.end(),
                "Failed to find filter with ID " + action.filter_id->ToString());

            (*filter_it)->components.UpdateBackBufferCopy([&](auto& comps_back) {
                auto comp_it = FindByID(comps_back, *action.component_id);
                if (comp_it != comps_back.end())
                {
                    comps_back.erase(comp_it);
                }
                else
                {
                    LOG_WARN(
                        "Failed to RemoveFilterComponent with component ID {} because it does not "
                        "exist",
                        *action.component_id);
                }
            });
        });
    });
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

        // Use the public API to check for empty and initialize
        if (application_state.filters.tabs.GetBack().empty())
        {
            auto default_tabs = DefaultState::MakeDefaultTabs();
            application_state.filters.tabs.Init(std::vector(default_tabs), std::move(default_tabs));
        }
    }
}

} // namespace Fluxion::Application::Layers::Actions::FiltersLayer
