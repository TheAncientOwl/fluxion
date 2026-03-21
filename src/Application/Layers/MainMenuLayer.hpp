/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MainMenuLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Render App's menu.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"
#include "Core/Application/Layers/TLayer.hpp"

namespace Fluxion::Application::Layers {

class MainMenuLayer : public Graphite::Core::Application::Layers::TLayer<AppState>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    MainMenuLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Core::Application::Layers::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

private:
    void RenderMenu();
};

} // namespace Fluxion::Application::Layers
