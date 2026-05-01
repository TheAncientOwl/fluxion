/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SettingsLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Implementation of @see SettingsLayer.hpp.
///

#include <cstdlib>

#include "IconsCodicons.h"
#include "imgui.h"

#include "DummyPlugin.hpp"
#include "Graphite/Logger.hpp"

#include "Modules/Theme.hpp"
#include "SettingsLayer.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::SettingsLayer);
USE_LOG_SCOPE(Fluxion::Application::Layers::SettingsLayer);

namespace Fluxion::Application::Layers {

std::string_view SettingsLayer::GetLayerName() noexcept
{
    return "SettingsLayer";
}

std::string_view SettingsLayer::GetName() const noexcept
{
    return SettingsLayer::GetLayerName();
}

std::string_view SettingsLayer::GetDisplayName() const noexcept
{
    return "Settings";
}

SettingsLayer::SettingsLayer(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TSoftMenuCloseableLayer{std::move(application), z_index}
{
    LOG_SCOPE("::SettingsLayer()");
}

bool SettingsLayer::IsActive() const noexcept
{
    return m_application->GetApplicationState().layers_active.settings;
}

void SettingsLayer::SetIsActive(bool active) noexcept
{
    m_application->GetApplicationState().layers_active.settings = active;
}

void SettingsLayer::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
    m_logs_plugin_renderer.OnAdd(m_application->GetApplicationState());
}

void SettingsLayer::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void SettingsLayer::OnRender()
{
    LOG_SCOPE("::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_SETTINGS_GEAR " Settings", &app_state.layers_active.settings);

    if (ImGui::BeginTabBar("SettingsTabBar"))
    {
        if (ImGui::BeginTabItem("Logs Plugins"))
        {
            m_logs_plugin_renderer.Render(app_state);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(ICON_CI_SYMBOL_COLOR " Theme"))
        {
            Modules::SettingsLayer::RenderTheme();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void SettingsLayer::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

} // namespace Fluxion::Application::Layers
