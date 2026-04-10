/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayerActions.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Main layer responsible for rendering logs table.
///

#include "LogsViewLayerActions.hpp"
#include "Fluxion/Application/Data/AppState.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::LogsViewLayer::Actions);
USE_LOG_SCOPE(Fluxion::Application::Layers::LogsViewLayer::Actions);

namespace Fluxion::Application::Layers::Actions::LogsViewLayer {

using namespace Fluxion::Application::Data;
using namespace Fluxion::Application::Data::Logs;

template <ELogsViewActionLayerType ActionType, typename TPayload>
void handle(AppState& application_state, TPayload const& payload) = delete;

template <>
void handle<ELogsViewActionLayerType::UpdateVisibleLogs>(
    AppState& application_state,
    LogsViewLayerActionPayload const& payload)
{
    LOG_SCOPE("::handle<UpdateVisibleLogs>()");
    // LOG_INFO("begin {} | end {}", action.visible_logs_indices.begin, action.visible_logs_indices.end);
    // TODO: resize the data when the imported logs change

    application_state.logs.visible_chunk.UpdateBackBufferSwap(
        // 1. Prepare Back Buffer
        [payload, columns_count = application_state.logs.table_header.size()](
            VisibleLogsChunk& visible_logs_chunk) {
            if (payload.visible_logs_indices.empty())
            {
                return;
            }

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
        [payload, &logs_logic = application_state.logs_plugin](VisibleLogsChunk& visible_logs_chunk) {
            logs_logic->GetLogs(
                payload.visible_logs_indices,
                Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter{visible_logs_chunk.logs});
        });
}

void HandleLogsViewLayersLayerAction(AppState& application_state, LogsViewLayerActionPayload const& action)
{
    LOG_SCOPE("::HandleLogsViewLayersLayerAction()");

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
        LOG_WARN(
            "::HandleLogsViewLayersLayerAction(): Unknown action type {}",
            static_cast<std::uint32_t>(action.type));
        break;
    }
    }
}

} // namespace Fluxion::Application::Layers::Actions::LogsViewLayer
