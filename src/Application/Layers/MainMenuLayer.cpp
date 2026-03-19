/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MainMenuLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Implementation of @see MainMenuLayer.hpp.
///

#include "MainMenuLayer.hpp"
#include "FiltersLayer.hpp"
#include "LogsViewLayer.hpp"

#include "icons/IconsCodicons.h"
#include "imgui/imgui.h"

#include "Api/DataIO.hpp"

namespace Fluxion::Application::Layers {

std::string_view MainMenuLayer::GetLayerName() noexcept
{
    return "MainMenuLayer";
}

std::string_view MainMenuLayer::GetName() const noexcept
{
    return MainMenuLayer::GetLayerName();
}

MainMenuLayer::MainMenuLayer(
    Fluxion::Application::FluxionApplication::Ptr application,
    Graphite::Core::Application::Layer::ZIndex const z_index)
    : BaseLayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void MainMenuLayer::OnAdd()
{
    LOG_SCOPE("");
}

void MainMenuLayer::OnRender()
{
    LOG_SCOPE("");

    RenderMenu();
}

void MainMenuLayer::OnRemove()
{
    LOG_SCOPE("");
}

void MainMenuLayer::RenderMenu()
{
    LOG_SCOPE("");
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(ICON_CI_SQUIRREL " Views"))
        {
            auto& app_state{m_application->GetApplicationState()};

            if (ImGui::MenuItem(
                    app_state.logs_view_open ? ICON_CI_EYE " Logs" : ICON_CI_EYE_CLOSED " LogsView"))
            {
                if (!app_state.logs_view_open)
                {
                    m_application->AddLayer<LogsViewLayer>(m_application->shared_from_this(), 10);
                }
                app_state.logs_view_open = !app_state.logs_view_open;
            }

            ImGui::Separator();

            if (ImGui::MenuItem(
                    app_state.filters.menu_open ? ICON_CI_EYE " Filters"
                                                : ICON_CI_EYE_CLOSED " Filters"))
            {
                if (!app_state.filters.menu_open)
                {
                    m_application->AddLayer<FiltersLayer>(m_application->shared_from_this(), 20);
                }
                app_state.filters.menu_open = !app_state.filters.menu_open;
            }

            ImGui::EndMenu();
        }

        // --- Right Side Stats ---
        // 1. Calculate how much space the FPS text will take
        char fps_text[32];
        snprintf(
            fps_text, sizeof(fps_text), "%.1f FPS", static_cast<double>(ImGui::GetIO().Framerate));

        float text_width = ImGui::CalcTextSize(fps_text).x;

        // 2. Set the cursor to the far right (minus the text width and some padding)
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - text_width - ImGui::GetStyle().ItemSpacing.x);

        // 3. Display it (use a color to make it stand out, maybe your leafGreen?)
        ImGui::TextColored(ImVec4(0.15f, 0.55f, 0.38f, 1.00f), "%s", fps_text);

        ImGui::EndMainMenuBar();
    }
}

} // namespace Fluxion::Application::Layers
