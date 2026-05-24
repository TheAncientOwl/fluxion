/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MainMenuView.cpp
/// @author Alexandru Delegeanu
/// @version 0.13
/// @brief Implementation of @see MainMenuView.hpp.
///

#include "MainMenuView.hpp"
#include "Graphite/Application/Views/TSoftCloseableView.hpp"
#include "Graphite/Logger.hpp"

#include <filesystem>

#include "IconsCodicons.h"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::MainMenuView);
USE_LOG_SCOPE(Fluxion::Application::Views::MainMenuView);

namespace Fluxion::Application::Views {

std::string_view MainMenuView::GetViewName() noexcept
{
    return "MainMenuView";
}

std::string_view MainMenuView::GetName() const noexcept
{
    return MainMenuView::GetViewName();
}

MainMenuView::MainMenuView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TView{std::move(application), render_priority}
{
    LOG_SCOPE("::MainMenuView()");
}

void MainMenuView::OnAdd()
{
    LOG_SCOPE("::OnAdd()");

    // Initialize last visited path
    m_last_file_dialog_path = std::filesystem::current_path();

    // Configure file dialog for importing logs
    std::vector<Graphite::Common::UI::FileFilter> log_filters = {
        {"Log Files", {".log", ".txt"}}, {"All Files", {".*"}}};
    m_file_dialog.SetSelectionCallback([this](const Graphite::Common::UI::FileDialogResult& result) {
        if (result.was_selected)
        {
            LOG_INFO("Selected log file: {}", result.path.string());
            m_last_file_dialog_path = result.path.parent_path();
            auto& app_state{m_application->GetApplicationState()};
            app_state.logs_plugin->ImportLogs(result.path);
            app_state.logs.table_header = app_state.logs_plugin->GetTableHeader();
        }
    });
}

void MainMenuView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void MainMenuView::OnRender()
{
    LOG_SCOPE("::OnRender()");

    m_file_dialog.Render();

    RenderMenu();
}

void MainMenuView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

void MainMenuView::RenderMenu()
{
    LOG_SCOPE("::RenderMenu()");
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(ICON_CI_CODE_OSS " File"))
        {
            if (ImGui::MenuItem(ICON_CI_ROCKET " Import Logs"))
            {
                m_file_dialog.Open(
                    "Select Log File to Import",
                    Graphite::Common::UI::EFileDialogMode::OpenFile,
                    m_last_file_dialog_path);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(ICON_CI_SQUIRREL " Views"))
        {
            m_application->ForEachView<
                Graphite::Application::Views::TSoftMenuCloseableView<AppState, EFluxionAction>>(
                [](Graphite::Application::Views::TSoftMenuCloseableView<AppState, EFluxionAction>& menu_item,
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

} // namespace Fluxion::Application::Views
