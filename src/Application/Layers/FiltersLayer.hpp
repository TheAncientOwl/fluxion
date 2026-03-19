/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Main layer responsible for rendering logs table.
///

#pragma once

#include "Application/AppState.hpp"
#include "Application/Fluxion.hpp"
#include "Core/Application/BaseLayer.hpp"

namespace Fluxion::Application::Layers {

class FiltersLayer : public Graphite::Core::Application::BaseLayer<AppState>
{
public: // Public API
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    FiltersLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Core::Application::Layer::ZIndex const z_index);

    void OnAdd() override;
    void OnRender() override;
    void OnRemove() override;

private: // Private Rendering API
    void RenderFiltersTabs();

    void RenderFiltersTab(std::shared_ptr<Fluxion::API::Data::FiltersTab> tab_ptr, bool& dirty);
    void RenderFilter(
        std::shared_ptr<Fluxion::API::Data::Filter> filter_ptr,
        std::shared_ptr<Fluxion::API::Data::FiltersTab> owning_tab_ptr,
        bool& dirty);
    void RenderFilterComponent(
        std::shared_ptr<Fluxion::API::Data::FilterComponent> component_ptr,
        std::shared_ptr<Fluxion::API::Data::Filter> owning_filter_ptr,
        bool& dirty);

private: // Private Logics API
    void HandleAction();

private: // Data Structures
    enum class EActionType : std::uint8_t
    {
        None = 0,

        AddFiltersTab = 1,
        RemoveFiltersTab = 2,
        DuplicateFiltersTab = 3,

        AddFilter = 4,
        RemoveFilter = 5,
        DuplicateFilter = 6,

        AddFilterComponent = 7,
        RemoveFilterComponent = 8,
    };

    struct Action
    {
        EActionType type{EActionType::None};
        std::shared_ptr<Fluxion::API::Data::FiltersTab> tab{nullptr};
        std::shared_ptr<Fluxion::API::Data::Filter> filter{nullptr};
        std::shared_ptr<Fluxion::API::Data::FilterComponent> component{nullptr};
    };

private: // Fields
    Action m_current_action{};
};

} // namespace Fluxion::Application::Layers
