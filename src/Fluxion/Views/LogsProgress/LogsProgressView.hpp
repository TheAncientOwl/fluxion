/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsProgressView.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Main view responsible for rendering import progress.
///

#pragma once

#include "Fluxion.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/Views/TView.hpp"

namespace Fluxion::Application::Views {

class LogsProgressView : public Graphite::Application::Views::TView<AppState, EFluxionAction>

{
public:
    static std::string_view GetViewName() noexcept;
    std::string_view GetName() const noexcept override;

    LogsProgressView(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Application::Views::RenderPriority const render_priority);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

    inline bool IsActive() const noexcept override;
    inline void SetIsActive(bool const open) override;
};

} // namespace Fluxion::Application::Views
