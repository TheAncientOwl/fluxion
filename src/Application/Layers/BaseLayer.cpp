/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BaseLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see BaseLayer.hpp.
///

#include "BaseLayer.hpp"
#include "FiltersLayer.hpp"
#include "LogsViewLayer.hpp"

#include "imgui/imgui.h"

namespace Fluxion::Application::Layers {

std::string_view BaseLayer::GetLayerName() noexcept
{
    return "BaseLayer";
}

std::string_view BaseLayer::GetName() const noexcept
{
    return BaseLayer::GetLayerName();
}

BaseLayer::BaseLayer(
    Fluxion::Application::FluxionApplication::Ptr application,
    Graphite::Core::Application::Layer::ZIndex const z_index)
    : ILayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void BaseLayer::OnAdd()
{
    LOG_SCOPE("");
}

void BaseLayer::OnRender()
{
    LOG_SCOPE("");
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);
}

void BaseLayer::OnRemove()
{
    LOG_SCOPE("");
}

} // namespace Fluxion::Application::Layers
