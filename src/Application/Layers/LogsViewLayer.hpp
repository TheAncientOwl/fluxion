/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"
#include "Core/Application/Layers/TSoftCloseableLayer.hpp"

namespace Fluxion::Application::Layers {

class LogsViewLayer : public Graphite::Core::Application::Layers::TSoftMenuCloseableLayer<AppState>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    LogsViewLayer(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Core::Application::Layers::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

    inline bool IsActive() const noexcept override;
    inline void SetIsActive(bool const open) override;
    inline std::string_view GetDisplayName() const noexcept override;

private:
    void RenderLogsTable();
};

} // namespace Fluxion::Application::Layers
