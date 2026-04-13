/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SettingsLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Settings UI layer for application configuration.
///

#pragma once

#include <filesystem>
#include <vector>

#include "Fluxion.hpp"
#include "Fluxion/Application/Data/AppState.hpp"
#include "Graphite/Application/Layers/TSoftCloseableLayer.hpp"

namespace Fluxion::Application::Layers {

class SettingsLayer
    : public Graphite::Application::Layers::TSoftMenuCloseableLayer<AppState, EFluxionAction>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;
    std::string_view GetDisplayName() const noexcept override;

    SettingsLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Application::Layers::ZIndex const z_index);

    bool IsActive() const noexcept override;
    void SetIsActive(bool active) noexcept override;

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

private:
    void RenderPluginSelection();
    void ScanAvailablePlugins();

    std::vector<std::filesystem::path> m_available_plugins;
    int m_selected_plugin_index{-1};
};

} // namespace Fluxion::Application::Layers
