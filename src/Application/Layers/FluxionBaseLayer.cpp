/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FluxionBaseLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Implementation of @see FluxionBaseLayer.hpp.
///

#include "FluxionBaseLayer.hpp"
#include "FiltersLayer.hpp"
#include "LogsViewLayer.hpp"

#include "imgui/imgui.h"

namespace Fluxion::Application::Layers {

std::string_view FluxionBaseLayer::GetLayerName() noexcept
{
    return "FluxionBaseLayer";
}

std::string_view FluxionBaseLayer::GetName() const noexcept
{
    return FluxionBaseLayer::GetLayerName();
}

FluxionBaseLayer::FluxionBaseLayer(
    Fluxion::Application::FluxionApplication::Ptr application,
    Graphite::Core::Application::Layer::ZIndex const z_index)
    : BaseLayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void FluxionBaseLayer::OnAdd()
{
    LOG_SCOPE("");
}

void FluxionBaseLayer::OnRender()
{
    LOG_SCOPE("");
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);
}

void FluxionBaseLayer::OnRemove()
{
    LOG_SCOPE("");
}

} // namespace Fluxion::Application::Layers
