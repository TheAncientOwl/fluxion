/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersView.hpp
/// @author Alexandru Delegeanu
/// @version 0.10
/// @brief Main view responsible for rendering logs table.
///

#pragma once

#include "FiltersViewActions.hpp"
#include "Fluxion.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/Views/TSoftCloseableView.hpp"
#include "Graphite/Application/Views/Utility/TDispatcher.hpp"

namespace Fluxion::Application::Views {

class FiltersView
    : public Graphite::Application::Views::TSoftCloseableView<AppState, EFluxionAction>
    , public Graphite::Application::Views::Utility::
          TDispatcher<FiltersView, EFluxionAction::FilterAction, Actions::FiltersView::FilterActionPayload>
{
public: // Public API
    static std::string_view GetViewName() noexcept;
    std::string_view GetName() const noexcept override;

    FiltersView(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Application::Views::RenderPriority const render_priority);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

    inline bool IsActive() const noexcept override;
    inline void SetIsActive(bool const open) override;
    inline std::string_view GetDisplayName() const noexcept override;

private: // Private API
    void MarkFiltersMetadataDirty();
    void MarkFiltersMetadataNotSavedOnDisk();

private: // Private Rendering API
    void RenderToolbar();
    void RenderTabs();

    void RenderTab(std::shared_ptr<Fluxion::Application::Data::Filters::Tab> tab_ptr);
    void RenderFilter(
        Graphite::Common::Utility::UniqueID const& owning_tab_id,
        Fluxion::Application::Data::Filters::Filter& filter);
    void RenderCondition(
        Graphite::Common::Utility::UniqueID const& owning_tab_id,
        Graphite::Common::Utility::UniqueID const& owning_filter_id,
        Fluxion::Application::Data::Filters::Condition& condition);
};

} // namespace Fluxion::Application::Views
