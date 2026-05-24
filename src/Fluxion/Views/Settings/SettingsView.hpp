/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SettingsView.hpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Settings UI view for application configuration.
///

#pragma once

#include "Fluxion.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/Views/TSoftCloseableView.hpp"
#include "Modules/LogsPlugin.hpp"

namespace Fluxion::Application::Views {

class SettingsView : public Graphite::Application::Views::TSoftCloseableView<AppState, EFluxionAction>
{
public:
    static std::string_view GetViewName() noexcept;
    std::string_view GetName() const noexcept override;
    std::string_view GetDisplayName() const noexcept override;

    SettingsView(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Application::Views::RenderPriority const render_priority);

    bool IsActive() const noexcept override;
    void SetIsActive(bool active) noexcept override;

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

private:
    Modules::SettingsView::LogsPluginRenderer m_logs_plugin_renderer{};
};

} // namespace Fluxion::Application::Views
