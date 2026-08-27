/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsTableViewActions.cpp
/// @author Alexandru Delegeanu
/// @version 0.10
/// @brief Main view responsible for rendering logs table.
///

#include "LogsTableViewActions.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Fluxion/Data/Formatters.hpp" // IWYU pragma: keep <for Range>
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::LogsTableView::Actions);
USE_LOG_SCOPE(Fluxion::Application::Views::LogsTableView::Actions);

namespace Fluxion::Application::Views::Actions::LogsTableView {

using namespace Fluxion::Application::Data;
using namespace Fluxion::Application::Data::Logs;

template <ELogsViewActionViewType ActionType, typename TPayload>
void handle(AppState& application_state, TPayload const& payload) = delete;

namespace Utility {

std::vector<Fluxion::API::LogsPlugin::Data::Range> MergeRanges(
    std::vector<Fluxion::API::LogsPlugin::Data::Range>& input_ranges)
{
    if (input_ranges.empty())
    {
        return {};
    }

    std::sort(input_ranges.begin(), input_ranges.end(), [](auto const& a, auto const& b) {
        if (a.begin != b.begin)
        {
            return a.begin < b.begin;
        }
        return a.end < b.end;
    });

    std::vector<Fluxion::API::LogsPlugin::Data::Range> merged;
    merged.push_back(input_ranges[0]);

    for (std::size_t idx = 1; idx < input_ranges.size(); ++idx)
    {
        auto& last = merged.back();
        auto const& current = input_ranges[idx];

        if (current.begin <= last.end)
        {
            last.end = std::max(last.end, current.end);
        }
        else
        {
            merged.push_back(current);
        }
    }

    return merged;
}

} // namespace Utility

template <>
void handle<ELogsViewActionViewType::UpdateVisibleLogs>(
    AppState& application_state,
    LogsTableViewActionPayload const& payload)
{
    LOG_SCOPE("::handle<UpdateVisibleLogs>()");
    // LOG_INFO("begin {} | end {}", action.visible_logs_indices.begin, action.visible_logs_indices.end);

    LOG_INFO("::handle<UpdateVisibleLogs>(): cleaning up visible logs indices");
    LOG_INFO("::handle<UpdateVisibleLogs>(): logs indices request: {}", payload.visible_logs_indices);
    auto final_request_indices{std::vector<Fluxion::API::LogsPlugin::Data::Range>{}};
    auto const& visible_logs{application_state.logs.visible.GetBack().logs};
    for (auto const& range : payload.visible_logs_indices)
    {
        if (!visible_logs.contains(range.begin) || !visible_logs.contains(range.end - 1))
        {
            final_request_indices.emplace_back(range.begin, range.end);
        }
    }
    final_request_indices = Utility::MergeRanges(final_request_indices);
    LOG_INFO("::handle<UpdateVisibleLogs>(): final logs indices request: {}", final_request_indices);

    application_state.logs.visible.UpdateBackBufferSwap(
        // 1. Prepare Back Buffer
        [payload, columns_count = application_state.logs.table_header.size()](
            VisibleLogs& visible_logs_chunk) {
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
                        if (idx >= interval.begin && idx < interval.end)
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
        [&final_request_indices,
         &logs_plugin = application_state.logs_plugin](VisibleLogs& visible_logs_chunk) {
            if (!final_request_indices.empty())
            {
                logs_plugin->GetLogs(
                    final_request_indices,
                    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter{visible_logs_chunk.logs});
            }
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
