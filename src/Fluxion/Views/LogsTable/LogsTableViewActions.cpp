/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsTableViewActions.cpp
/// @author Alexandru Delegeanu
/// @version 0.8
/// @brief Main view responsible for rendering logs table.
///

#include "LogsTableViewActions.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::LogsTableView::Actions);
USE_LOG_SCOPE(Fluxion::Application::Views::LogsTableView::Actions);

namespace Fluxion::Application::Views::Actions::LogsTableView {

using namespace Fluxion::Application::Data;
using namespace Fluxion::Application::Data::Logs;

template <ELogsViewActionViewType ActionType, typename TPayload>
void handle(AppState& application_state, TPayload const& payload) = delete;

template <>
void handle<ELogsViewActionViewType::UpdateVisibleLogs>(
    AppState& application_state,
    LogsTableViewActionPayload const& payload)
{
    LOG_SCOPE("::handle<UpdateVisibleLogs>()");
    // LOG_INFO("begin {} | end {}", action.visible_logs_indices.begin, action.visible_logs_indices.end);
    // TODO: resize the data when the imported logs change

    application_state.logs.visible.UpdateBackBufferSwap(
        // 1. Prepare Back Buffer
        [payload, columns_count = application_state.logs.table_header.size()](
            VisibleLogs& visible_logs_chunk) {
            if (payload.visible_logs_indices.empty())
            {
                return;
            }

            // TODO: better handle logs request / culling so we don't hit the plugin for the same chunks every frame
            LOG_INFO(
                "::handle<UpdateVisibleLogs>(): buffer_preparer > Begin culling. Map size: {}",
                visible_logs_chunk.logs.size());

            std::erase_if(
                visible_logs_chunk.logs, [&indices = payload.visible_logs_indices](auto const& item) {
                    const auto idx = item.first;
                    // Check if idx is inside any valid interval
                    for (auto const& interval : indices)
                    {
                        if (idx >= interval.begin && idx <= interval.end)
                        {
                            return false; // Keep it
                        }
                    }
                    return true; // Cull it
                });

            LOG_INFO(
                "::handle<UpdateVisibleLogs>(): buffer_preparer > Culling complete. Map size: {}",
                visible_logs_chunk.logs.size());
        },
        // 2. Update Back Buffer
        [payload, &logs_logic = application_state.logs_plugin](VisibleLogs& visible_logs_chunk) {
            logs_logic->GetLogs(
                payload.visible_logs_indices,
                Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter{visible_logs_chunk.logs});
        });
}

void HandleLogsTableViewsViewAction(AppState& application_state, LogsTableViewActionPayload const& action)
{
    LOG_SCOPE("::HandleLogsTableViewsViewAction()");

    if (action.type == ELogsViewActionViewType::None)
    {
        return;
    }

    switch (action.type)
    {
    case ELogsViewActionViewType::UpdateVisibleLogs: {
        handle<ELogsViewActionViewType::UpdateVisibleLogs>(application_state, action);
        break;
    }
    default: {
        LOG_WARN(
            "::HandleLogsTableViewsViewAction(): Unknown action type {}",
            static_cast<std::uint32_t>(action.type));
        break;
    }
    }
}

} // namespace Fluxion::Application::Views::Actions::LogsTableView
