/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DevView.hpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief Debug menus.
///

#pragma once

#include "Fluxion.hpp"
#include "Fluxion/Data/AppState.hpp"

#include "Graphite/Application/Views/TSoftCloseableView.hpp"

namespace Fluxion::Application::Views {

class DevView : public Graphite::Application::Views::TSoftCloseableView<AppState, EFluxionAction>
{
public:
    static std::string_view GetViewName() noexcept;
    std::string_view GetName() const noexcept override;

    DevView(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Application::Views::RenderPriority const render_priority);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

    inline bool IsActive() const noexcept override;
    inline void SetIsActive(bool const open) override;
    inline std::string_view GetDisplayName() const noexcept override;

private:
    void RenderLogger();
};

} // namespace Fluxion::Application::Views
