/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BaseLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief Render App's menu.
///

#pragma once

#include "Fluxion.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/Layers/TLayer.hpp"

namespace Fluxion::Application::Layers {

class BaseLayer : public Graphite::Application::Layers::TLayer<AppState, EFluxionAction>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    BaseLayer(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Application::Layers::ZIndex const z_index);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;
};

} // namespace Fluxion::Application::Layers
