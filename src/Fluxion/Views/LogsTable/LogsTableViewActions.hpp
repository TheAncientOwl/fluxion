/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsTableViewActions.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Actions of @see LogsTableView.hpp
///

#pragma once

#include "Fluxion/Data/AppState.hpp"

namespace Fluxion::Application::Views::Actions::LogsTableView {

enum class ELogsViewActionViewType : std::uint8_t
{
    None = 0,
    UpdateVisibleLogs = 1,
};

struct LogsTableViewActionPayload
{
    ELogsViewActionViewType type{ELogsViewActionViewType::None};
    std::vector<Fluxion::API::LogsPlugin::Bridge::Range> visible_logs_indices{};
};

void HandleLogsTableViewsViewAction(AppState& application_state, LogsTableViewActionPayload const& action);

} // namespace Fluxion::Application::Views::Actions::LogsTableView
