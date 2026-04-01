/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayerActions.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
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
    LOG_INFO("begin {} | end {}", action.visible_logs_indices.begin, action.visible_logs_indices.end);
    // TODO: resize the data when the imported logs change

    application_state.logs.visible_chunk.UpdateBackBufferSwap(
        // 1. Prepare Back Buffer
        [action, columns_count = application_state.logs.table_header.size()](
            VisibleLogsChunk& visible_logs_chunk) {
            auto& logs_chunk = visible_logs_chunk.data;

            if (auto const new_size = static_cast<size_t>(
                    action.visible_logs_indices.end - action.visible_logs_indices.begin);
                new_size > logs_chunk.size())
            {
                auto const old_size = logs_chunk.size();
                logs_chunk.resize(new_size);

                for (size_t row_idx = old_size; row_idx < new_size; ++row_idx)
                {
                    logs_chunk[row_idx].resize(columns_count);
                }
            }
        },
        // 2. Update Back Buffer
        [action, &logs_logic = application_state.logs_logic](VisibleLogsChunk& visible_logs_chunk) {
            auto& logs_chunk = visible_logs_chunk.data;

            visible_logs_chunk.filled_size = logs_logic->GetLogsChunk(
                static_cast<size_t>(action.visible_logs_indices.begin),
                static_cast<size_t>(action.visible_logs_indices.end),
                logs_chunk);
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
