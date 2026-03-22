/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DevLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Debug menus.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"

#include "Graphite/Application/Layers/TSoftCloseableLayer.hpp"

namespace Fluxion::Application::Layers {

class DevLayer : public Graphite::Application::Layers::TSoftMenuCloseableLayer<AppState>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    DevLayer(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Application::Layers::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

    inline bool IsActive() const noexcept override;
    inline void SetIsActive(bool const open) override;
    inline std::string_view GetDisplayName() const noexcept override;

private:
    void RenderLogger();
};

} // namespace Fluxion::Application::Layers
