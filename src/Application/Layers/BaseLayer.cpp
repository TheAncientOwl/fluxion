/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BaseLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Implementation of @see BaseLayer.hpp.
///

#include "BaseLayer.hpp"

#include "Graphite/Logger.hpp"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::BaseLayer);
USE_LOG_SCOPE(Fluxion::Application::Layers::BaseLayer);

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
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TLayer{std::move(application), z_index}
{
    LOG_SCOPE("::BaseLayer()");
}

void BaseLayer::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
}

void BaseLayer::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void BaseLayer::OnRender()
{
    LOG_SCOPE("::OnRender()");
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);
}

void BaseLayer::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

} // namespace Fluxion::Application::Layers
