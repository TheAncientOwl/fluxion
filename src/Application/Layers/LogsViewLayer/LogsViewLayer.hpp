/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include "AppState.hpp"
#include "Fluxion.hpp"
#include "Graphite/Application/Layers/TSoftCloseableLayer.hpp"
#include "Graphite/Application/Layers/Utility/TDispatcher.hpp"
#include "LogsViewLayerActions.hpp"

namespace Fluxion::Application::Layers {

class LogsViewLayer
    : public Graphite::Application::Layers::TSoftMenuCloseableLayer<AppState, EFluxionAction>
    , public Graphite::Application::Layers::Utility::
          TDispatcher<LogsViewLayer, EFluxionAction::LogsViewLayerAction, Actions::LogsViewLayer::LogsViewLayerActionPayload>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    LogsViewLayer(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Application::Layers::ZIndex const z_index);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

    inline bool IsActive() const noexcept override;
    inline void SetIsActive(bool const open) override;
    inline std::string_view GetDisplayName() const noexcept override;

private:
    void RenderLogsTable();
};

} // namespace Fluxion::Application::Layers
