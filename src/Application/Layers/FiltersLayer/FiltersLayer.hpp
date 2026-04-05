/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.9
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include "AppState.hpp"
#include "FiltersLayerActions.hpp"
#include "Fluxion.hpp"
#include "Graphite/Application/Layers/TSoftCloseableLayer.hpp"
#include "Graphite/Application/Layers/Utility/TDispatcher.hpp"

namespace Fluxion::Application::Layers {

class FiltersLayer
    : public Graphite::Application::Layers::TSoftMenuCloseableLayer<AppState, EFluxionAction>
    , public Graphite::Application::Layers::Utility::
          TDispatcher<FiltersLayer, EFluxionAction::FilterAction, Actions::FiltersLayer::FilterActionPayload>
{
public: // Public API
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    FiltersLayer(
        FluxionApplication::FluxionApplication::Ptr application,
        Graphite::Application::Layers::ZIndex const z_index);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

    inline bool IsActive() const noexcept override;
    inline void SetIsActive(bool const open) override;
    inline std::string_view GetDisplayName() const noexcept override;

private: // Private API
    void MarkFiltersMetadataDirty();

private: // Private Rendering API
    void RenderToolbar();
    void RenderTabs();

    void RenderTab(std::shared_ptr<Fluxion::API::Data::Filters::Tab> tab_ptr);
    void RenderFilter(
        Graphite::Common::Utility::UniqueID const& owning_tab_id,
        Fluxion::API::Data::Filters::Filter& filter);
    void RenderCondition(
        Graphite::Common::Utility::UniqueID const& owning_tab_id,
        Graphite::Common::Utility::UniqueID const& owning_filter_id,
        Fluxion::API::Data::Filters::Condition& condition);
};

} // namespace Fluxion::Application::Layers
