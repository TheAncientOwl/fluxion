/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsTableView.hpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Main view responsible for rendering logs table.
///

#pragma once

#include "Fluxion.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/Views/TSoftCloseableView.hpp"
#include "Graphite/Application/Views/Utility/TDispatcher.hpp"
#include "LogsTableViewActions.hpp"

namespace Fluxion::Application::Views {

class LogsTableView
    : public Graphite::Application::Views::TSoftMenuCloseableView<AppState, EFluxionAction>
    , public Graphite::Application::Views::Utility::
          TDispatcher<LogsTableView, EFluxionAction::LogsTableViewAction, Actions::LogsTableView::LogsTableViewActionPayload>
{
public:
    static std::string_view GetViewName() noexcept;
    std::string_view GetName() const noexcept override;

    LogsTableView(
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
    void RenderLogsTable();
};

} // namespace Fluxion::Application::Views
