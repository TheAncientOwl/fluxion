/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.7
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
    auto& app_state{m_application->GetApplicationState()};
    auto const headers{app_state.logs_logic->GetTableHeader()};

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

        std::for_each(headers.cbegin(), headers.cend(), [](auto const& header) {
            ImGui::TableSetupColumn(header.c_str());
        });
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper{};
        clipper.Begin(static_cast<int>(app_state.logs_logic->GetTotalLogs()));

        while (clipper.Step())
        {
            LOG_INFO("Rendering from {} to {}", clipper.DisplayStart, clipper.DisplayEnd - 1);

            auto const logs_chunk = app_state.logs_logic->GetLogsChunk(
                static_cast<std::size_t>(clipper.DisplayStart),
                static_cast<std::size_t>(clipper.DisplayEnd));
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
