/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.13
/// @brief Main layer responsible for rendering logs table.
///

#include <algorithm>

#include "FiltersLayerActions.hpp"
#include "Fluxion/Application/Data/Formatters.hpp" // IWYU pragma: keep
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::FiltersLayer::Actions);
USE_LOG_SCOPE(Fluxion::Application::Layers::FiltersLayer::Actions);

namespace Fluxion::Application::Layers::Actions::FiltersLayer {

using namespace Fluxion::Application::Data;
using namespace Fluxion::Application::Data::Filters;

template <typename T>
auto FindByID(std::vector<std::shared_ptr<T>>& vec, Graphite::Common::Utility::UniqueID const& id)
{
    return std::find_if(vec.begin(), vec.end(), [&](auto const& ptr) { return ptr->id == id; });
}

template <EFilterActionType ActionType, typename TPayload>
void handle(AppState& application_state, TPayload const& payload) = delete;

template <>
void handle<EFilterActionType::AddTab>(AppState& application_state, Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<AddTab>()");

    GRAPHITE_ASSERT(payload.tab_id == std::nullopt, "tab should be nullopt for AddTab payload");
    GRAPHITE_ASSERT(
        payload.filter_id == std::nullopt, "filter should be nullopt for AddTab payload");
    GRAPHITE_ASSERT(
        payload.condition_id == std::nullopt, "condition should be nullopt for AddTab payload");

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
void handle<EFilterActionType::RemoveTab>(
    AppState& application_state,
    Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<RemoveTab>()");

    GRAPHITE_ASSERT(
        payload.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveTab payload");
    GRAPHITE_ASSERT(
        payload.filter_id == std::nullopt, "filter should be nullopt for RemoveTab payload");
    GRAPHITE_ASSERT(
        payload.condition_id == std::nullopt, "condition should be nullopt for RemoveTab payload");

    application_state.filters.tabs.UpdateBackBufferCopy([&](std::vector<Tab::Ptr>& tabs_back) {
        auto const tab_it = FindByID(tabs_back, *payload.tab_id);
        if (tab_it != tabs_back.end())
        {
            tabs_back.erase(tab_it);
        }
        else
        {
            LOG_WARN(
                "::handle<RemoveTab>(): Failed to RemoveTab with tab ID {} because it does not "
                "exist",
                *payload.tab_id);
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
void handle<EFilterActionType::DuplicateTab>(
    AppState& application_state,
    Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<DuplicateTab>()");
    GRAPHITE_ASSERT(
        payload.tab_id != std::nullopt, "tab should NOT be nullopt for DuplicateTab payload");
    GRAPHITE_ASSERT(
        payload.filter_id == std::nullopt, "filter should be nullopt for DuplicateTab payload");
    GRAPHITE_ASSERT(
        payload.condition_id == std::nullopt,
        "condition should be nullopt for DuplicateTab payload");

    Fluxion::Application::AppState::IdToMetadataMapUpdates id_to_metadata_updates{};

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto const tab_it = FindByID(tabs_back, *payload.tab_id);
        if (tab_it == tabs_back.end())
        {
            LOG_WARN(
                "::handle<DuplicateTab>(): Failed to DuplicateTab with tab ID {} because it does "
                "not "
                "exist",
                *payload.tab_id);
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
            id_to_metadata_updates.emplace_back(
                filter_dup->id, Fluxion::API::Data::Common::Highlight{filter_dup->colors});

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

    application_state.filters.id_to_metadata_updates.UpdateBackBufferSwap(
        [](auto& /**/) {},
        [&](Fluxion::Application::AppState::IdToMetadataMapUpdates& back_updates) {
            back_updates = std::move(id_to_metadata_updates);
        });
}

template <>
void handle<EFilterActionType::AddFilter>(
    AppState& application_state,
    Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<AddFilter>()");
    GRAPHITE_ASSERT(
        payload.tab_id != std::nullopt, "tab should NOT be nullopt for AddFilter payload");
    GRAPHITE_ASSERT(
        payload.filter_id == std::nullopt, "filter should be nullopt for AddFilter payload");
    GRAPHITE_ASSERT(
        payload.condition_id == std::nullopt, "condition should be nullopt for AddFilter payload");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *payload.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + payload.tab_id->ToString());

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
void handle<EFilterActionType::RemoveFilter>(
    AppState& application_state,
    Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<RemoveFilter>()");
    GRAPHITE_ASSERT(
        payload.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveFilter payload");
    GRAPHITE_ASSERT(
        payload.filter_id != std::nullopt, "filter should NOT be nullopt for RemoveFilter payload");
    GRAPHITE_ASSERT(
        payload.condition_id == std::nullopt,
        "condition should be nullopt for RemoveFilter payload");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *payload.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + payload.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *payload.filter_id);
            if (filter_it != filters_back.end())
            {
                filters_back.erase(filter_it);
            }
            else
            {
                LOG_WARN(
                    "::handle<RemoveFilter>(): Failed to RemoveFilter with filter ID {} because it "
                    "does not exist",
                    *payload.filter_id);
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
void handle<EFilterActionType::DuplicateFilter>(
    AppState& application_state,
    Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<DuplicateFilter>()");
    GRAPHITE_ASSERT(
        payload.tab_id != std::nullopt, "tab should NOT be nullopt for DuplicateFilter payload");
    GRAPHITE_ASSERT(
        payload.filter_id != std::nullopt,
        "filter should NOT be nullopt for DuplicateFilter payload");
    GRAPHITE_ASSERT(
        payload.condition_id == std::nullopt,
        "condition should be nullopt for DuplicateFilter payload");

    Fluxion::Application::AppState::IdToMetadataMapUpdates id_to_metadata_updates{};

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *payload.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + payload.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *payload.filter_id);
            if (filter_it == filters_back.end())
            {
                LOG_WARN(
                    "::handle<DuplicateFilter>(): Failed to DuplicateFilter with filter ID {} "
                    "because it does not exist",
                    *payload.filter_id);
                return;
            }

            auto duplicate_filter = std::make_shared<Filter>(**filter_it);
            duplicate_filter->id = Graphite::Common::Utility::UniqueID::Generate();
            duplicate_filter->name += "*";
            id_to_metadata_updates.emplace_back(
                duplicate_filter->id, Fluxion::API::Data::Common::Highlight{duplicate_filter->colors});

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

    application_state.filters.id_to_metadata_updates.UpdateBackBufferSwap(
        [](auto& /**/) {},
        [&](Fluxion::Application::AppState::IdToMetadataMapUpdates& back_updates) {
            back_updates = std::move(id_to_metadata_updates);
        });
}

template <>
void handle<EFilterActionType::AddCondition>(
    AppState& application_state,
    Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<AddCondition>()");
    GRAPHITE_ASSERT(
        payload.tab_id != std::nullopt, "tab should NOT be nullopt for AddCondition payload");
    GRAPHITE_ASSERT(
        payload.filter_id != std::nullopt, "filter should NOT be nullopt for AddCondition payload");
    GRAPHITE_ASSERT(
        payload.condition_id == std::nullopt,
        "condition should be nullopt for AddCondition payload");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *payload.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + payload.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *payload.filter_id);
            GRAPHITE_ASSERT(
                filter_it != filters_back.end(),
                "Failed to find filter with ID " + payload.filter_id->ToString());

            (*filter_it)->conditions.UpdateBackBufferCopy([&](auto& comps_back) {
                auto new_comp = comps_back.emplace_back(std::make_shared<Condition>());
                new_comp->id = Graphite::Common::Utility::UniqueID::Generate();
                (*new_comp)[EConditionFlag::IsEquals] = true;
            });
        });
    });
}

template <>
void handle<EFilterActionType::RemoveCondition>(
    AppState& application_state,
    Payloads::FiltersDataModify const& payload)
{
    LOG_SCOPE("::handle<RemoveCondition>()");
    GRAPHITE_ASSERT(
        payload.tab_id != std::nullopt, "tab should NOT be nullopt for RemoveCondition payload");
    GRAPHITE_ASSERT(
        payload.filter_id != std::nullopt,
        "filter should NOT be nullopt for RemoveCondition payload");
    GRAPHITE_ASSERT(
        payload.condition_id != std::nullopt,
        "condition should NOT be nullopt for RemoveCondition payload");

    application_state.filters.tabs.UpdateBackBufferCopy([&](auto& tabs_back) {
        auto tab_it = FindByID(tabs_back, *payload.tab_id);
        GRAPHITE_ASSERT(
            tab_it != tabs_back.end(), "Failed to find tab with ID " + payload.tab_id->ToString());

        (*tab_it)->filters.UpdateBackBufferCopy([&](auto& filters_back) {
            auto filter_it = FindByID(filters_back, *payload.filter_id);
            GRAPHITE_ASSERT(
                filter_it != filters_back.end(),
                "Failed to find filter with ID " + payload.filter_id->ToString());

            (*filter_it)->conditions.UpdateBackBufferCopy([&](auto& comps_back) {
                auto comp_it = FindByID(comps_back, *payload.condition_id);
                if (comp_it != comps_back.end())
                {
                    comps_back.erase(comp_it);
                }
                else
                {
                    LOG_WARN(
                        "::handle<RemoveCondition>(): Failed to RemoveCondition with condition ID "
                        "{} "
                        "because it does not "
                        "exist",
                        *payload.condition_id);
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
void handle<EFilterActionType::ApplyFilters>(AppState& application_state, int const& /* no-payload */)
{
    LOG_SCOPE("::handle<ApplyFilters>()");
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters{};
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> highlight_only{};

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

            std::vector<Fluxion::API::LogsPlugin::Data::Condition> out_conditions{};
            out_conditions.reserve(filter.conditions.GetBack().size());
            for (auto const& condition_ptr : filter.conditions.GetBack())
            {
                auto const& condition{*condition_ptr};
                auto& out_condition = out_conditions.emplace_back();
                out_condition.column_index = get_column_index(condition.over_column_id);
                out_condition.data = condition.data;

                using EInternalConditionFlag = EConditionFlag;
                using EBridgeConditionFlag = Fluxion::API::LogsPlugin::Data::EConditionFlag;

                out_condition[EBridgeConditionFlag::IsRegex] =
                    condition[EInternalConditionFlag::IsRegex];
                out_condition[EBridgeConditionFlag::IsEquals] =
                    condition[EInternalConditionFlag::IsEquals];
                out_condition[EBridgeConditionFlag::IsCaseSensitive] =
                    condition[EInternalConditionFlag::IsCaseSensitive];
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

    LOG_INFO("::handle<ApplyFilters>(): Active filters size: {}", filters.size());
    LOG_INFO("::handle<ApplyFilters>(): HighlightOnly-Active filters size: {}", highlight_only.size());

    application_state.logs_plugin->ApplyFilters(std::move(filters), std::move(highlight_only));

    application_state.logs.searched_log.UpdateBackBufferCopyLocking(
        [](Data::Logs::SearchedLog& searched_log) { searched_log.index = std::nullopt; });
}

template <>
void handle<EFilterActionType::DisableFilters>(AppState& application_state, int const& /* no-payload */)
{
    LOG_SCOPE("::handle<DisableFilters>()");
    application_state.logs_plugin->DisableFilters();
    application_state.logs.searched_log.UpdateBackBufferCopyLocking(
        [](Data::Logs::SearchedLog& searched_log) { searched_log.index = std::nullopt; });
}

template <>
void handle<EFilterActionType::NextLog>(AppState& application_state, Payloads::SearchLog const& payload)
{
    LOG_SCOPE("::handle<NextLog>()");
    application_state.logs.searched_log.UpdateBackBufferCopyLocking(
        [&](Data::Logs::SearchedLog& searched_log) {
            searched_log.index = application_state.logs_plugin->GetNextLog(payload.filter_id);
            LOG_INFO("::handle<NextLog>(): log index == {}", searched_log.index);
        });
}

template <>
void handle<EFilterActionType::PrevLog>(AppState& application_state, Payloads::SearchLog const& payload)
{
    LOG_SCOPE("::handle<PrevLog>()");
    application_state.logs.searched_log.UpdateBackBufferCopyLocking(
        [&](Data::Logs::SearchedLog& searched_log) {
            searched_log.index = application_state.logs_plugin->GetPrevLog(payload.filter_id);
            LOG_INFO("::handle<PrevLog>(): log index == {}", searched_log.index);
        });
}

void HandleFiltersLayerAction(AppState& application_state, FilterActionPayload const& payload)
{
    static auto constexpr c_no_payload{0};

    if (payload.type == EFilterActionType::None)
    {
        return;
    }

    LOG_TRACE("::HandleFiltersLayerAction(): type {}", static_cast<std::uint32_t>(payload.type));

    switch (payload.type)
    {
    case EFilterActionType::ApplyFilters: {
        handle<EFilterActionType::ApplyFilters>(application_state, c_no_payload);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) { metadata[EFiltersMetadataFlag::Applied] = true; });
        break;
    }
    case EFilterActionType::DisableFilters: {
        handle<EFilterActionType::DisableFilters>(application_state, c_no_payload);
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) { metadata[EFiltersMetadataFlag::Applied] = false; });
        break;
    }

    case EFilterActionType::NextLog: {
        handle<EFilterActionType::NextLog>(
            application_state, std::get<Payloads::SearchLog>(payload.data));
        break;
    };
    case EFilterActionType::PrevLog: {
        handle<EFilterActionType::PrevLog>(
            application_state, std::get<Payloads::SearchLog>(payload.data));
        break;
    };

    case EFilterActionType::AddTab: {
        handle<EFilterActionType::AddTab>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::RemoveTab: {
        handle<EFilterActionType::RemoveTab>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::DuplicateTab: {
        handle<EFilterActionType::DuplicateTab>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::AddFilter: {
        handle<EFilterActionType::AddFilter>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::RemoveFilter: {
        handle<EFilterActionType::RemoveFilter>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::DuplicateFilter: {
        handle<EFilterActionType::DuplicateFilter>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::AddCondition: {
        handle<EFilterActionType::AddCondition>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    case EFilterActionType::RemoveCondition: {
        handle<EFilterActionType::RemoveCondition>(
            application_state, std::get<Payloads::FiltersDataModify>(payload.data));
        application_state.filters.metadata.UpdateBackBufferCopyLocking(
            [](FiltersGeneralMetadata& metadata) {
                metadata[EFiltersMetadataFlag::Applied] = false;
                metadata[EFiltersMetadataFlag::SavedToDisk] = false;
            });
        break;
    }
    default: {
        LOG_ERROR(
            "::HandleFiltersLayerAction(): Unknown payload type {}",
            static_cast<std::uint32_t>(payload.type));
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
