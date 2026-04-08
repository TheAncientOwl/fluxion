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

#include "Fluxion/Application/Data/AppState.hpp"

namespace Fluxion::Application::Layers::Actions::FiltersLayer {

enum class EFilterActionType : std::uint8_t
{
    None = 0,
    ApplyFilters,
    DisableFilters,
    AddTab,
    RemoveTab,
    DuplicateTab,
    AddFilter,
    RemoveFilter,
    DuplicateFilter,
    AddCondition,
    RemoveCondition
};

struct FilterActionPayload
{
    EFilterActionType type{EFilterActionType::None};
    std::optional<Graphite::Common::Utility::UniqueID> tab_id{};
    std::optional<Graphite::Common::Utility::UniqueID> filter_id{};
    std::optional<Graphite::Common::Utility::UniqueID> condition_id{};
};

void HandleFiltersLayerAction(AppState& application_state, FilterActionPayload const& action);

} // namespace Fluxion::Application::Layers::Actions::FiltersLayer
