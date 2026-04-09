/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DevLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.11
/// @brief Implementation of @see DevLayer.hpp.
///

#include "DevLayer.hpp"
#include "Graphite/Logger.hpp"
#include "Modules/Logger.hpp"
#include "Modules/Theme.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::DevLayer);
USE_LOG_SCOPE(Fluxion::Application::Layers::DevLayer);

namespace Fluxion::Application::Layers {

std::string_view DevLayer::GetLayerName() noexcept
{
    return "DevLayer";
}

std::string_view DevLayer::GetName() const noexcept
{
    return DevLayer::GetLayerName();
}

DevLayer::DevLayer(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TSoftMenuCloseableLayer{std::move(application), z_index}
{
    LOG_SCOPE("::DevLayer::DevLayer()");
}

void DevLayer::OnAdd()
{
    LOG_SCOPE("::DevLayer::OnAdd()");
}

void DevLayer::OnIterate()
{
    LOG_SCOPE("::DevLayer::OnIterate()");
}

void DevLayer::OnRender()
{
    LOG_SCOPE("::DevLayer::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_SYMBOL_EVENT " Dev", &app_state.layers_active.debug);

    if (ImGui::BeginTabBar("Dev"))
    {
        if (ImGui::BeginTabItem(ICON_CI_OUTPUT " Logger"))
        {
            Modules::DevLayer::RenderLogger();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(ICON_CI_SYMBOL_COLOR " Theme"))
        {
            Modules::DevLayer::RenderTheme();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void DevLayer::OnRemove()
{
    LOG_SCOPE("::DevLayer::OnRemove()");
}

inline bool DevLayer::IsActive() const noexcept
{
    return m_application->GetApplicationState().layers_active.debug;
}

inline void DevLayer::SetIsActive(bool const open)
{
    m_application->GetApplicationState().layers_active.debug = open;
}

inline std::string_view DevLayer::GetDisplayName() const noexcept
{
    return "Debug";
}

} // namespace Fluxion::Application::Layers
