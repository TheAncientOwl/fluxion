/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsViewLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"
#include "Core/Application/ILayer.hpp"

namespace Fluxion::Application::Layers {

class LogsViewLayer : public Graphite::Core::Application::ILayer<AppState>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    LogsViewLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Core::Application::Layer::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

private:
    void RenderLogsTable();
};

} // namespace Fluxion::Application::Layers
