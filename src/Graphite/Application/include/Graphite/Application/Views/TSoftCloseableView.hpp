/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TSoftMenuCloseableView.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief View that is removed / added on close / open.
///

#pragma once

#include "TView.hpp"

namespace Graphite::Application::Views {

template <typename ApplicationState, typename ActionEnum>
class TSoftMenuCloseableView : public TView<ApplicationState, ActionEnum>
{
public:
    virtual inline std::string_view GetDisplayName() const noexcept = 0;

    virtual bool IsActive() const noexcept = 0;
    virtual void SetIsActive(bool active) = 0;

    TSoftMenuCloseableView(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>> application,
        RenderPriority const render_priority)
        : TView<ApplicationState, ActionEnum>(std::move(application), render_priority)
    {
    }

    TSoftMenuCloseableView(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>> application,
        RenderPriority const render_priority,
        Graphite::Common::Utility::UniqueID id)
        : TView<ApplicationState, ActionEnum>(std::move(application), render_priority, std::move(id))
    {
    }
};

} // namespace Graphite::Application::Views
