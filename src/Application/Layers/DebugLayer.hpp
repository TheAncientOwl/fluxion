/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DebugLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Debug menus.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"
#include "Core/Application/BaseLayer.hpp"

namespace Fluxion::Application::Layers {

class DebugLayer : public Graphite::Core::Application::BaseLayer<AppState>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    DebugLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Core::Application::Layer::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

private:
    void RenderLogger();
};

} // namespace Fluxion::Application::Layers
