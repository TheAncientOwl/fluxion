/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.10
/// @brief Main layer responsible for rendering logs table.
///

#include <algorithm>

#include "FiltersLayerActions.hpp"
#include "Graphite/Logger.hpp"

namespace Fluxion::Application::Layers::Actions::FiltersLayer {

using namespace Fluxion::API::Data::Filters;

template <typename T>
auto FindByID(std::vector<std::shared_ptr<T>>& vec, Graphite::Common::Utility::UniqueID const& id)
{
    return std::find_if(vec.begin(), vec.end(), [&](auto const& ptr) { return ptr->id == id; });
}

template <EFilterActionType ActionType>
void handle(AppState& application_state, FilterActionPayload const& action) = delete;
template <>
void handle<EFilterActionType::AddTab>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(action.tab_id == std::nullopt, "tab should be nullopt for AddTab action");
    GRAPHITE_ASSERT(action.filter_id == std::nullopt, "filter should be nullopt for AddTab action");
    GRAPHITE_ASSERT(
        action.condition_id == std::nullopt, "condition should be nullopt for AddTab action");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_ptr = tabs_back.emplace_back(std::make_shared<Tab>());
        tab_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
        tab_ptr->name = "New Tab";
        (*tab_ptr)[ETabFlag::IsActive] = true;
        tab_ptr->UpdateImGuiID();

        // Initialize nested structures
        auto filter_ptr = std::make_shared<Filter>();
        filter_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
        filter_ptr->name = "New Filter";
        filter_ptr->colors = {
            .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
        (*filter_ptr)[EFilterFlag::IsActive] = true;

        auto comp_ptr = std::make_shared<Condition>();
        comp_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
        (*comp_ptr)[EConditionFlag::IsEquals] = true;

        filter_ptr->conditions.Init({comp_ptr}, {comp_ptr});
        tab_ptr->filters.Init({filter_ptr}, {filter_ptr});
    });
}

template <>
void handle<EFilterActionType::RemoveTab>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(action.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveTab action");
    GRAPHITE_ASSERT(
        action.filter_id == std::nullopt, "filter should be nullopt for RemoveTab action");
    GRAPHITE_ASSERT(
        action.condition_id == std::nullopt, "condition should be nullopt for RemoveTab action");

    application_state.filters.tabs.UpdateBackBufferCopy(
        [&](std::vector<Fluxion::API::Data::Filters::Tab::Ptr>& tabs_back) {
            auto const tab_it = FindByID(tabs_back, *action.tab_id);
            if (tab_it != tabs_back.end())
            {
                tabs_back.erase(tab_it);
            }
            else
            {
                LOG_WARN(
                    "Failed to RemoveTab with tab ID {} because it does not exist", *action.tab_id);
            }
            if (tabs_back.empty())
            {
                // Create a new Tab
                auto tab_ptr = tabs_back.emplace_back(std::make_shared<Tab>());
                tab_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
                tab_ptr->name = "New Tab";
                (*tab_ptr)[ETabFlag::IsActive] = true;
                tab_ptr->UpdateImGuiID();

                // Initialize nested structures: one Filter with one Condition
                auto filter_ptr = std::make_shared<Filter>();
                filter_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
                filter_ptr->name = "New Filter";
                filter_ptr->colors = {
                    .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
                (*filter_ptr)[EFilterFlag::IsActive] = true;

                auto comp_ptr = std::make_shared<Condition>();
                comp_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
                (*comp_ptr)[EConditionFlag::IsEquals] = true;

                filter_ptr->conditions.Init({comp_ptr}, {comp_ptr});
                tab_ptr->filters.Init({filter_ptr}, {filter_ptr});
            }
            tabs_back.shrink_to_fit();
        });
}

template <>
void handle<EFilterActionType::DuplicateTab>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for DuplicateTab action");
    GRAPHITE_ASSERT(
        action.filter_id == std::nullopt, "filter should be nullopt for DuplicateTab action");
    GRAPHITE_ASSERT(
        action.condition_id == std::nullopt, "condition should be nullopt for DuplicateTab action");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto const tab_it = FindByID(tabs_back, *action.tab_id);
        if (tab_it == tabs_back.end())
        {
            LOG_WARN("Failed to DuplicateTab with tab ID {} because it does not exist", *action.tab_id);
            return;
        }

        auto duplicated_tab = std::make_shared<Tab>(**tab_it);
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
            // TODO: fix shared style duplication in a thread-safe manner
            filter_dup->colors = {};

            auto current_comps = filter_dup->conditions.GetBack();
            std::vector<Condition::Ptr> new_comps_list;
            for (auto& comp_ptr : current_comps)
            {
                auto comp_dup = std::make_shared<Condition>(*comp_ptr);
                comp_dup->id = Graphite::Common::Utility::UniqueID::Generate();
                new_comps_list.push_back(std::move(comp_dup));
            }
            filter_dup->conditions.Init(std::vector(new_comps_list), std::move(new_comps_list));
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
        action.condition_id == std::nullopt, "condition should be nullopt for AddFilter action");

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

            auto comp_ptr = std::make_shared<Condition>();
            comp_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
            (*comp_ptr)[EConditionFlag::IsEquals] = true;
            new_filter->conditions.Init({comp_ptr}, {comp_ptr});
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
        action.condition_id == std::nullopt, "condition should be nullopt for RemoveFilter action");

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

            if (filters_back.empty())
            {
                auto filter_ptr = filters_back.emplace_back(std::make_shared<Filter>());
                filter_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
                filter_ptr->name = "New Filter";
                filter_ptr->colors = {
                    .foreground = {1.0f, 1.0f, 1.0f, 1.0f}, .background = {0.0f, 0.0f, 0.0f, 0.25f}};
                (*filter_ptr)[EFilterFlag::IsActive] = true;

                auto comp_ptr = std::make_shared<Condition>();
                comp_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
                (*comp_ptr)[EConditionFlag::IsEquals] = true;
                filter_ptr->conditions.Init({comp_ptr}, {comp_ptr});
            }
            filters_back.shrink_to_fit();
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
        action.condition_id == std::nullopt,
        "condition should be nullopt for DuplicateFilter action");

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
            // TODO: fix shared style duplication in a thread-safe manner
            duplicate_filter->colors = {};

            auto current_comps = duplicate_filter->conditions.GetBack();
            std::vector<Condition::Ptr> new_comps_list;
            for (auto& comp_ptr : current_comps)
            {
                auto comp_dup = std::make_shared<Condition>(*comp_ptr);
                comp_dup->id = Graphite::Common::Utility::UniqueID::Generate();
                new_comps_list.push_back(std::move(comp_dup));
            }
            duplicate_filter->conditions.Init(std::vector(new_comps_list), std::move(new_comps_list));

            filters_back.insert(std::next(filter_it), std::move(duplicate_filter));
        });
    });
}

template <>
void handle<EFilterActionType::AddCondition>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for AddCondition action");
    GRAPHITE_ASSERT(
        action.filter_id != std::nullopt, "filter should NOT be nullopt for AddCondition action");
    GRAPHITE_ASSERT(
        action.condition_id == std::nullopt, "condition should be nullopt for AddCondition action");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *action.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + action.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *action.filter_id);
            GRAPHITE_ASSERT(
                filter_it != filters_back.end(),
                "Failed to find filter with ID " + action.filter_id->ToString());

            (*filter_it)->conditions.UpdateBackBufferCopy([&](auto& comps_back) {
                auto new_comp = comps_back.emplace_back(std::make_shared<Condition>());
                new_comp->id = Graphite::Common::Utility::UniqueID::Generate();
                (*new_comp)[EConditionFlag::IsEquals] = true;
            });
        });
    });
}

template <>
void handle<EFilterActionType::RemoveCondition>(AppState& application_state, FilterActionPayload const& action)
{
    GRAPHITE_ASSERT(
        action.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveCondition action");
    GRAPHITE_ASSERT(
        action.filter_id != std::nullopt, "filter should NOT be nullopt for RemoveCondition action");
    GRAPHITE_ASSERT(
        action.condition_id != std::nullopt,
        "condition should NOT be nullopt for RemoveCondition action");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *action.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + action.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *action.filter_id);
            GRAPHITE_ASSERT(
                filter_it != filters_back.end(),
                "Failed to find filter with ID " + action.filter_id->ToString());

            (*filter_it)->conditions.UpdateBackBufferCopy([&](auto& comps_back) {
                auto comp_it = FindByID(comps_back, *action.condition_id);
                if (comp_it != comps_back.end())
                {
                    comps_back.erase(comp_it);
                }
                else
                {
                    LOG_WARN(
                        "Failed to RemoveCondition with condition ID {} because it does not "
                        "exist",
                        *action.condition_id);
                }

                if (comps_back.empty())
                {
                    auto comp_ptr = comps_back.emplace_back(std::make_shared<Condition>());
                    comp_ptr->id = Graphite::Common::Utility::UniqueID::Generate();
                    (*comp_ptr)[EConditionFlag::IsEquals] = true;
                }
                comps_back.shrink_to_fit();
            });
        });
    });
}

template <>
void handle<EFilterActionType::ApplyFilters>(AppState& application_state, FilterActionPayload const& /* action */)
{
    LOG_SCOPE("");
    std::vector<Fluxion::API::Data::Filters::Active::Filter> filters{};
    std::vector<Fluxion::API::Data::Filters::Active::Filter> highlight_only{};

    auto const& tabs{application_state.filters.tabs.GetBack()};
    auto const& header{application_state.logs_plugin->GetTableHeader()};

    auto get_column_index = [&header](Graphite::Common::Utility::UniqueID const id) {
        for (std::size_t index = 0; index < header.size(); ++index)
        {
            if (header[index].id == id)
            {
                return index;
            }
        }

        GRAPHITE_ASSERT(false, std::string{"Failed to find header with ID "} + id);

        return std::size_t{0};
    };

    for (auto const& tab_ptr : tabs)
    {
        auto const& tab{*tab_ptr};
        if (!tab[ETabFlag::IsActive])
        {
            continue;
        }

        for (auto const& filter_ptr : tab.filters.GetBack())
        {
            auto const& filter{*filter_ptr};
            if (!filter[EFilterFlag::IsActive] || filter.conditions.GetBack().empty())
            {
                continue;
            }

            std::vector<Fluxion::API::Data::Filters::Active::Condition> out_conditions{};
            out_conditions.reserve(filter.conditions.GetBack().size());
            for (auto const& condition_ptr : filter.conditions.GetBack())
            {
                auto const& condition{*condition_ptr};
                auto& out_condition = out_conditions.emplace_back();
                out_condition.column_index = get_column_index(condition.over_column_id);
                out_condition.data = condition.data;

                out_condition[EConditionFlag::IsRegex] = condition[EConditionFlag::IsRegex];
                out_condition[EConditionFlag::IsEquals] = condition[EConditionFlag::IsEquals];
                out_condition[EConditionFlag::IsCaseSensitive] =
                    condition[EConditionFlag::IsCaseSensitive];
            }

            if (filter[EFilterFlag::IsHighlightOnly])
            {
                highlight_only.emplace_back(
                    filter.id, std::move(out_conditions), filter.colors, filter.priority);
            }
            else
            {
                filters.emplace_back(
                    filter.id, std::move(out_conditions), filter.colors, filter.priority);
            }
        }
    }

    std::sort(filters.begin(), filters.end(), [](auto const& a, auto const& b) {
        return a.priority > b.priority;
    });
    std::sort(highlight_only.begin(), highlight_only.end(), [](auto const& a, auto const& b) {
        return a.priority > b.priority;
    });

    LOG_DEBUG("Active filters size: {}", filters.size());
    LOG_DEBUG("HighlightOnly-Active filters size: {}", highlight_only.size());

    application_state.logs_plugin->ApplyFilters(std::move(filters), std::move(highlight_only));
}

template <>
void handle<EFilterActionType::DisableFilters>(AppState& application_state, FilterActionPayload const& /* action */)
{
    LOG_SCOPE("");
    application_state.logs_plugin->DisableFilters();
}

void HandleFiltersLayerAction(AppState& application_state, FilterActionPayload const& action)
{
    if (action.type == EFilterActionType::None)
    {
        return;
    }

    LOG_TRACE(
        "Handling action type {} -> tab: {} | filter: {} | condition: {}",
        static_cast<std::uint32_t>(action.type),
        action.tab_id ? action.tab_id->ToString() : "nullopt",
        action.filter_id ? action.filter_id->ToString() : "nullopt",
        action.condition_id ? action.condition_id->ToString() : "nullopt");

    switch (action.type)
    {
    case EFilterActionType::ApplyFilters: {
        handle<EFilterActionType::ApplyFilters>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = true;
            });
        break;
    }
    case EFilterActionType::DisableFilters: {
        handle<EFilterActionType::DisableFilters>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
            });
        break;
    }

    case EFilterActionType::AddTab: {
        handle<EFilterActionType::AddTab>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::RemoveTab: {
        handle<EFilterActionType::RemoveTab>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::DuplicateTab: {
        handle<EFilterActionType::DuplicateTab>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::AddFilter: {
        handle<EFilterActionType::AddFilter>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::RemoveFilter: {
        handle<EFilterActionType::RemoveFilter>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::DuplicateFilter: {
        handle<EFilterActionType::DuplicateFilter>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::AddCondition: {
        handle<EFilterActionType::AddCondition>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::RemoveCondition: {
        handle<EFilterActionType::RemoveCondition>(application_state, action);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](Internal::FiltersGeneralMetadata& metadata) {
                metadata[Internal::EFiltersMetadataFlag::Applied] = false;
                metadata[Internal::EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    default: {
        LOG_WARN(
            "Unknown action type {} -> tab: {} | filter: {} | condition: {}",
            static_cast<std::uint32_t>(action.type),
            action.tab_id ? action.tab_id->ToString() : "nullopt",
            action.filter_id ? action.filter_id->ToString() : "nullopt",
            action.condition_id ? action.condition_id->ToString() : "nullopt");
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
