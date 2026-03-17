/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Implementation of @see LogsViewLayer.hpp.
///

#include "LogsViewLayer.hpp"

#include "imgui/imgui.h"

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
    Fluxion::Application::FluxionApplication::Ptr application,
    Graphite::Core::Application::Layer::ZIndex const z_index)
    : ILayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void LogsViewLayer::OnAdd()
{
    LOG_SCOPE("");
}

void LogsViewLayer::OnRender()
{
    LOG_SCOPE("");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin("LogsView", &app_state.logs_view_open);

    RenderLogsTable();

    ImGui::End();

    if (!m_application->GetApplicationState().logs_view_open)
    {
        m_application->RemoveLayer(m_layer_id);
    }
}

void LogsViewLayer::RenderLogsTable()
{
    auto& app_state{m_application->GetApplicationState()};
    auto const headers{app_state.logs_logic->GetTableHeader()};

    ImGui::BeginChild("LogsTableRegion");
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 5));
    if (ImGui::BeginTable(
            "LogsTable",
            headers.size(),
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable |
                ImGuiTableFlags_SizingFixedFit,
            ImVec2(0.0f, 0.0f)))
    {
        ImGui::TableSetupScrollFreeze(1, 1);

        std::for_each(headers.cbegin(), headers.cend(), [](auto const& header) {
            ImGui::TableSetupColumn(header.c_str());
        });
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper{};
        clipper.Begin(static_cast<int>(app_state.logs_logic->GetTotalLogs()));

        while (clipper.Step())
        {
            LOG_INFO("Rendering from {} to {}", clipper.DisplayStart, clipper.DisplayEnd - 1);

            auto const logs_chunk =
                app_state.logs_logic->GetLogsChunk(clipper.DisplayStart, clipper.DisplayEnd);
            for (auto const& log_row : logs_chunk)
            {
                ImGui::TableNextRow();

                for (auto const& log_field : log_row)
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", log_field.c_str());
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
