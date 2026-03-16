/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BaseLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Render App's menu.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"
#include "Core/Application/ILayer.hpp"

namespace Fluxion::Application::Layers {

class BaseLayer : public Graphite::Core::Application::ILayer<AppState>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    BaseLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Core::Application::Layer::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

private:
};

} // namespace Fluxion::Application::Layers
