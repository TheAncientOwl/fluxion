/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayerActions.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Actions of @see LogsViewLayer.hpp
///

#pragma once

#include "Fluxion/Data/AppState.hpp"

namespace Fluxion::Application::Layers::Actions::LogsViewLayer {

enum class ELogsViewActionLayerType : std::uint8_t
{
    None = 0,
    UpdateVisibleLogs = 1,
};

struct LogsViewLayerActionPayload
{
    ELogsViewActionLayerType type{ELogsViewActionLayerType::None};
    std::vector<Fluxion::API::LogsPlugin::Data::Range> visible_logs_indices{};
};

void HandleLogsViewLayersLayerAction(AppState& application_state, LogsViewLayerActionPayload const& action);

} // namespace Fluxion::Application::Layers::Actions::LogsViewLayer
