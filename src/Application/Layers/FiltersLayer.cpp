/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see FiltersLayer.hpp.
///

#include "FiltersLayer.hpp"

#include "imgui/imgui.h"

#include "Api/DataIO.hpp"

namespace Fluxion::Application::Layers {

std::string_view FiltersLayer::GetLayerName() noexcept
{
    return "FiltersLayer";
}

std::string_view FiltersLayer::GetName() const noexcept
{
    return FiltersLayer::GetLayerName();
}

FiltersLayer::FiltersLayer(
    Fluxion::Application::FluxionApplication::Ptr application,
    Graphite::Core::Application::Layer::ZIndex const z_index)
    : ILayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void FiltersLayer::OnAdd()
{
    LOG_SCOPE("");
}

void FiltersLayer::OnRender()
{
    LOG_SCOPE("");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin("Filters", &app_state.filters.menu_open, ImGuiWindowFlags_NoTitleBar);

    RenderFilterTabs();

    ImGui::End();

    if (!m_application->GetApplicationState().filters.menu_open)
    {
        m_application->RemoveLayer(m_layer_uid);
    }
}

void FiltersLayer::OnRemove()
{
    LOG_SCOPE("");
}

void FiltersLayer::RenderFilterTabs()
{
    LOG_SCOPE("");
    auto& app_state{m_application->GetApplicationState()};
    LOG_DEBUG("Filter Tabs: {}", Fluxion::Utils::Format::format_vector(app_state.filters.data.tabs));
    LOG_DEBUG(
        "Filter Filters: {}", Fluxion::Utils::Format::format_vector(app_state.filters.data.filters));
    LOG_DEBUG(
        "Filter Components: {}",
        Fluxion::Utils::Format::format_vector(app_state.filters.data.components));

    if (ImGui::BeginTabBar("Tabs"))
    {
        if (ImGui::BeginTabItem("Tab 1"))
        {
            ImGui::Text("Content of Tab 1");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Tab 2"))
        {
            ImGui::Text("Content of Tab 2");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

} // namespace Fluxion::Application::Layers
