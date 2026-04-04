/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayerActions.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Main layer responsible for rendering logs table.
///

#include "LogsViewLayerActions.hpp"
#include "AppState.hpp"
#include "Graphite/Logger.hpp"

namespace Fluxion::Application::Layers::Actions::LogsViewLayer {

template <ELogsViewActionLayerType ActionType>
void handle(AppState& application_state, LogsViewLayerActionPayload const& action) = delete;

template <>
void handle<ELogsViewActionLayerType::UpdateVisibleLogs>(
    AppState& application_state,
    LogsViewLayerActionPayload const& action)
{
    LOG_SCOPE("");
    // LOG_INFO("begin {} | end {}", action.visible_logs_indices.begin, action.visible_logs_indices.end);
    // TODO: resize the data when the imported logs change

    application_state.logs.visible_chunk.UpdateBackBufferSwap(
        // 1. Prepare Back Buffer
        [action, columns_count = application_state.logs.table_header.size()](
            VisibleLogsChunk& visible_logs_chunk) {
            if (action.visible_logs_indices.empty())
            {
                return;
            }

            LOG_DEBUG("Begin culling. Map size: {}", visible_logs_chunk.logs.size());

            std::erase_if(
                visible_logs_chunk.logs, [&indices = action.visible_logs_indices](auto const& item) {
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

            LOG_DEBUG("Culling complete. Map size: {}", visible_logs_chunk.logs.size());
        },
        // 2. Update Back Buffer
        [action, &logs_logic = application_state.logs_logic](VisibleLogsChunk& visible_logs_chunk) {
            logs_logic->GetLogsChunk(
                action.visible_logs_indices,
                Fluxion::API::Data::Logs::IndexToLogRowMapWriter{visible_logs_chunk.logs});
        });
}

void HandleLogsViewLayersLayerAction(AppState& application_state, LogsViewLayerActionPayload const& action)
{
    LOG_SCOPE("");

    if (action.type == ELogsViewActionLayerType::None)
    {
        return;
    }

    switch (action.type)
    {
    case ELogsViewActionLayerType::UpdateVisibleLogs: {
        handle<ELogsViewActionLayerType::UpdateVisibleLogs>(application_state, action);
        break;
    }
    default: {
        LOG_WARN("Unknown action type {}", static_cast<std::uint32_t>(action.type));
        break;
    }
    }
}

} // namespace Fluxion::Application::Layers::Actions::LogsViewLayer
