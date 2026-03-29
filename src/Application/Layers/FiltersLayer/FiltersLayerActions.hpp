/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include "AppState.hpp"

namespace Fluxion::Application::Layers::Actions::FiltersLayer {

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

void HandleFiltersLayerAction(AppState& application_state, FilterActionPayload const& action);

} // namespace Fluxion::Application::Layers::Actions::FiltersLayer
