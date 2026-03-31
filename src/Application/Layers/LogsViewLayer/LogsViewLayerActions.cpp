/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayerActions.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
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

    auto& logs_chunk = application_state.logs.visible_chunk.back.data;

    if (auto const new_size =
            static_cast<size_t>(action.visible_logs_indices.end - action.visible_logs_indices.begin);
        new_size > logs_chunk.size())
    {
        auto const old_size = logs_chunk.size();
        logs_chunk.resize(new_size);

        for (size_t row_idx = old_size; row_idx < new_size; ++row_idx)
        {
            logs_chunk[row_idx].resize(application_state.logs.table_header.size());
        }
    }

    application_state.logs.visible_chunk.back.filled_size = application_state.logs_logic->GetLogsChunk(
        static_cast<size_t>(action.visible_logs_indices.begin),
        static_cast<size_t>(action.visible_logs_indices.end),
        logs_chunk);

    application_state.logs.visible_chunk.MarkDirty();
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
