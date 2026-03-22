/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MainMenuLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.9
/// @brief Implementation of @see MainMenuLayer.hpp.
///

#include "MainMenuLayer.hpp"
#include "DebugLayer/DebugLayer.hpp"
#include "FiltersLayer.hpp"
#include "Graphite/Application/Layers/TSoftCloseableLayer.hpp"
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
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TLayer{std::move(application), z_index}
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
            m_application->ForEachLayer<Graphite::Application::Layers::TSoftMenuCloseableLayer<AppState>>(
                [](Graphite::Application::Layers::TSoftMenuCloseableLayer<AppState>& menu_item,
                   bool const is_last) {
                    static char display_name_buffer[64];

                    const char* icon = menu_item.IsActive() ? ICON_CI_EYE : ICON_CI_EYE_CLOSED;
                    std::snprintf(
                        display_name_buffer,
                        sizeof(display_name_buffer),
                        "%s %.*s",
                        icon,
                        static_cast<int>(menu_item.GetDisplayName().size()),
                        menu_item.GetDisplayName().data());

                    if (ImGui::MenuItem(display_name_buffer))
                    {
                        menu_item.SetIsActive(!menu_item.IsActive());
                    }

                    if (!is_last)
                    {
                        ImGui::Separator();
                    }
                });

            ImGui::EndMenu();
        }

        // --- Right Side Stats ---
        // 1. Calculate how much space the FPS text will take
        char fps_text[16];
        snprintf(
            fps_text, sizeof(fps_text), "%.1f FPS", static_cast<double>(ImGui::GetIO().Framerate));

        auto const text_width = ImGui::CalcTextSize(fps_text).x;

        // 2. Set the cursor to the far right (minus the text width and some padding)
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - text_width - ImGui::GetStyle().ItemSpacing.x);

        // 3. Display it (use a color to make it stand out, maybe your leafGreen?)
        ImGui::TextColored(ImVec4(0.15f, 0.55f, 0.38f, 1.00f), "%s", fps_text);

        ImGui::EndMainMenuBar();
    }
}

} // namespace Fluxion::Application::Layers
