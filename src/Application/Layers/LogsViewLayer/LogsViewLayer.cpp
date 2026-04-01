/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.11
/// @brief Implementation of @see LogsViewLayer.hpp.
///

#include "LogsViewLayer.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

namespace Fluxion::Application::Layers {

std::string_view LogsViewLayer::GetLayerName() noexcept
{
    return "LogsViewLayer";
}

std::string_view LogsViewLayer::GetName() const noexcept
{
    return LogsViewLayer::GetLayerName();
}

LogsViewLayer::LogsViewLayer(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TSoftMenuCloseableLayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void LogsViewLayer::OnAdd()
{
    LOG_SCOPE("");
}

void LogsViewLayer::OnIterate()
{
    LOG_SCOPE("");
    m_application->GetApplicationState().logs.visible_chunk.SyncFrontBufferSwap();
}

void LogsViewLayer::OnRender()
{
    LOG_SCOPE("");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_OUTPUT " Logs", &app_state.layers_active.logs_view);

    RenderLogsTable();

    ImGui::End();
}

inline bool LogsViewLayer::IsActive() const noexcept
{
    return m_application->GetApplicationState().layers_active.logs_view;
}

inline void LogsViewLayer::SetIsActive(bool const open)
{
    m_application->GetApplicationState().layers_active.logs_view = open;
}

inline std::string_view LogsViewLayer::GetDisplayName() const noexcept
{
    return "Logs";
}

void LogsViewLayer::RenderLogsTable()
{
    LOG_SCOPE("");
    auto& app_state{m_application->GetApplicationState()};
    auto const& headers{app_state.logs_logic->GetTableHeader()};

    ImGui::BeginChild("LogsTableRegion");
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 5));
    if (ImGui::BeginTable(
            "LogsTable",
            static_cast<int>(headers.size()),
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable |
                ImGuiTableFlags_SizingFixedFit,
            ImVec2(0.0f, 0.0f)))
    {
        ImGui::TableSetupScrollFreeze(1, 1);

        for (auto const& header : headers)
        {
            ImGui::TableSetupColumn(header.c_str());
        }
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper{};
        clipper.Begin(static_cast<int>(app_state.logs_logic->GetTotalLogs()));

        while (clipper.Step())
        {
            // 1. Dispatch the request for the currently visible range
            Dispatch(
                Actions::LogsViewLayer::LogsViewLayerActionPayload{
                    .type = Actions::LogsViewLayer::ELogsViewActionLayerType::UpdateVisibleLogs,
                    .visible_logs_indices = {.begin = clipper.DisplayStart, .end = clipper.DisplayEnd}});
            LOG_INFO("Requested update begin {} | end {}", clipper.DisplayStart, clipper.DisplayEnd);

            // 2. Calculate exactly how many rows ImGui expects us to render this step
            auto const rows_to_render =
                static_cast<std::size_t>(clipper.DisplayEnd - clipper.DisplayStart);
            LOG_INFO("Rendering {}* rows", rows_to_render);

            // 3. Render EXACTLY that many rows
            for (std::size_t i = 0; i < rows_to_render; ++i)
            {
                ImGui::TableNextRow();

                // Check if we have valid data in the 'front' buffer for this relative row.
                // Because of double-buffering, there might be a 1-frame delay where
                // filled_size doesn't match rows_to_render.
                auto const& front_buffer = app_state.logs.visible_chunk.GetFront();
                if (i < front_buffer.filled_size)
                {
                    for (auto const& log_field : front_buffer.data[i])
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(log_field.c_str());
                    }
                }
                else
                {
                    // Fallback: Render empty cells to maintain table structure and Y-cursor math
                    for (size_t col = 0; col < headers.size(); ++col)
                    {
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted("...");
                    }
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void LogsViewLayer::OnRemove()
{
    LOG_SCOPE("");
}

} // namespace Fluxion::Application::Layers
