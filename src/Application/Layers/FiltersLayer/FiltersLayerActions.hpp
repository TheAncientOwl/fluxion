/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include <variant>

#include "Fluxion/Application/Data/AppState.hpp"

namespace Fluxion::Application::Layers::Actions::FiltersLayer {

enum class EFilterActionType : std::uint8_t
{
    None = 0,
    ApplyFilters,
    DisableFilters,

    NextLog,
    PrevLog,

    AddTab,
    RemoveTab,
    DuplicateTab,
    MoveFilter,
    AddFilter,
    RemoveFilter,
    DuplicateFilter,
    AddCondition,
    RemoveCondition,
    MoveCondition
};

namespace Payloads {

struct FiltersDataModify
{
    std::optional<Graphite::Common::Utility::UniqueID> tab_id{};
    std::optional<Graphite::Common::Utility::UniqueID> filter_id{};
    std::optional<Graphite::Common::Utility::UniqueID> condition_id{};
};

struct SearchLog
{
    Graphite::Common::Utility::UniqueID filter_id{Graphite::Common::Utility::UniqueID::Default()};
};

struct MoveFilter
{
    Graphite::Common::Utility::UniqueID tab_id{Graphite::Common::Utility::UniqueID::Default()};
    Graphite::Common::Utility::UniqueID filter_id{Graphite::Common::Utility::UniqueID::Default()};
    Graphite::Common::Utility::UniqueID target_filter_id{
        Graphite::Common::Utility::UniqueID::Default()};
};

struct MoveCondition
{
    Graphite::Common::Utility::UniqueID tab_id{Graphite::Common::Utility::UniqueID::Default()};
    Graphite::Common::Utility::UniqueID filter_id{Graphite::Common::Utility::UniqueID::Default()};
    Graphite::Common::Utility::UniqueID condition_id{Graphite::Common::Utility::UniqueID::Default()};
    Graphite::Common::Utility::UniqueID target_condition_id{
        Graphite::Common::Utility::UniqueID::Default()};
};

}; // namespace Payloads

struct FilterActionPayload
{
    EFilterActionType type{EFilterActionType::None};
    std::variant<Payloads::FiltersDataModify, Payloads::SearchLog, Payloads::MoveFilter, Payloads::MoveCondition>
        data{};
};

void HandleFiltersLayerAction(AppState& application_state, FilterActionPayload const& action);

} // namespace Fluxion::Application::Layers::Actions::FiltersLayer
