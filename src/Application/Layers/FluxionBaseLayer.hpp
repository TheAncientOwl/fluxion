/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FluxionBaseLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Render App's menu.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"
#include "Core/Application/BaseLayer.hpp"

namespace Fluxion::Application::Layers {

class FluxionBaseLayer : public Graphite::Core::Application::BaseLayer<AppState>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    FluxionBaseLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Core::Application::Layer::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

private:
};

} // namespace Fluxion::Application::Layers
