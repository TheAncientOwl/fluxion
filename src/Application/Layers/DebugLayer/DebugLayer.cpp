/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DebugLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.8
/// @brief Implementation of @see DebugLayer.hpp.
///

#include <algorithm>
#include <regex>

#include "Application/Layers/FiltersLayer.hpp"
#include "Application/Layers/LogsViewLayer.hpp"
#include "DebugLayer.hpp"
#include "Modules/Logger.hpp"

#include "icons/IconsCodicons.h"
#include "imgui/imgui.h"

namespace Fluxion::Application::Layers {

std::string_view DebugLayer::GetLayerName() noexcept
{
    return "DebugLayer";
}

std::string_view DebugLayer::GetName() const noexcept
{
    return DebugLayer::GetLayerName();
}

DebugLayer::DebugLayer(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TSoftMenuCloseableLayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void DebugLayer::OnAdd()
{
    LOG_SCOPE("");
}

void DebugLayer::OnRender()
{
    LOG_SCOPE("");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_SYMBOL_EVENT " Debug", &app_state.layers_active.debug);

    if (ImGui::BeginTabBar("Debug"))
    {
        if (ImGui::BeginTabItem(ICON_CI_OUTPUT " Logger"))
        {
            Modules::DebugLayer::RenderLogger();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void DebugLayer::OnRemove()
{
    LOG_SCOPE("");
}

inline bool DebugLayer::IsActive() const noexcept
{
    return m_application->GetApplicationState().layers_active.debug;
}

inline void DebugLayer::SetIsActive(bool const open)
{
    m_application->GetApplicationState().layers_active.debug = open;
}

inline std::string_view DebugLayer::GetDisplayName() const noexcept
{
    return "Debug";
}

} // namespace Fluxion::Application::Layers
