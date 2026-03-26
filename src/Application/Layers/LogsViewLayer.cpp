/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.8
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

        for (const auto& header : headers)
        {
            ImGui::TableSetupColumn(header.c_str());
        }
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper{};
        clipper.Begin(static_cast<int>(app_state.logs_logic->GetTotalLogs()));

        // TODO: resize the data when the imported logs change
        static std::vector<std::vector<std::string>> s_logs_chunk{};

        while (clipper.Step())
        {
            const size_t new_size = static_cast<size_t>(clipper.DisplayEnd - clipper.DisplayStart);

            if (new_size > s_logs_chunk.size())
            {
                auto const old_size = s_logs_chunk.size();
                s_logs_chunk.resize(new_size);

                for (size_t row_idx = old_size; row_idx < new_size; ++row_idx)
                {
                    s_logs_chunk[row_idx].resize(headers.size());
                }
            }

            auto const filled_size = app_state.logs_logic->GetLogsChunk(
                static_cast<size_t>(clipper.DisplayStart),
                static_cast<size_t>(clipper.DisplayEnd),
                s_logs_chunk);

            for (size_t log_idx = 0, size = std::min(new_size, filled_size); log_idx < size; ++log_idx)
            {
                ImGui::TableNextRow();
                for (const auto& log_field : s_logs_chunk[log_idx])
                {
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(log_field.c_str());
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
