/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SettingsView.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Implementation of @see SettingsView.hpp.
///

#include <cstdlib>

#include "IconsCodicons.h"
#include "imgui.h"

#include "Graphite/Logger.hpp"

#include "Modules/Theme.hpp"
#include "SettingsView.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::SettingsView);
USE_LOG_SCOPE(Fluxion::Application::Views::SettingsView);

namespace Fluxion::Application::Views {

std::string_view SettingsView::GetViewName() noexcept
{
    return "SettingsView";
}

std::string_view SettingsView::GetName() const noexcept
{
    return SettingsView::GetViewName();
}

std::string_view SettingsView::GetDisplayName() const noexcept
{
    return "Settings";
}

SettingsView::SettingsView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TSoftCloseableView{std::move(application), render_priority}
{
    LOG_SCOPE("::SettingsView()");
}

bool SettingsView::IsActive() const noexcept
{
    return m_application->GetApplicationState().views_active.settings;
}

void SettingsView::SetIsActive(bool active) noexcept
{
    m_application->GetApplicationState().views_active.settings = active;
}

void SettingsView::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
    m_logs_plugin_renderer.OnAdd(m_application->GetApplicationState());
}

void SettingsView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void SettingsView::OnRender()
{
    LOG_SCOPE("::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_SETTINGS_GEAR " Settings", &app_state.views_active.settings);

    if (ImGui::BeginTabBar("SettingsTabBar"))
    {
        if (ImGui::BeginTabItem("Logs Plugins"))
        {
            m_logs_plugin_renderer.Render(app_state);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(ICON_CI_SYMBOL_COLOR " Theme"))
        {
            Modules::SettingsView::RenderTheme();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void SettingsView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

} // namespace Fluxion::Application::Views
