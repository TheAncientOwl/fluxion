/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DevView.cpp
/// @author Alexandru Delegeanu
/// @version 0.12
/// @brief Implementation of @see DevView.hpp.
///

#include "DevView.hpp"
#include "Graphite/Logger.hpp"
#include "Modules/Logger.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::DevView);
USE_LOG_SCOPE(Fluxion::Application::Views::DevView);

namespace Fluxion::Application::Views {

std::string_view DevView::GetViewName() noexcept
{
    return "DevView";
}

std::string_view DevView::GetName() const noexcept
{
    return DevView::GetViewName();
}

DevView::DevView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TSoftMenuCloseableView{std::move(application), render_priority}
{
    LOG_SCOPE("::DevView()");
}

void DevView::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
}

void DevView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void DevView::OnRender()
{
    LOG_SCOPE("::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    ImGui::Begin(ICON_CI_SYMBOL_EVENT " Dev", &app_state.views_active.debug);

    if (ImGui::BeginTabBar("Dev"))
    {
        if (ImGui::BeginTabItem(ICON_CI_OUTPUT " Logger"))
        {
            Modules::DevView::RenderLogger();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void DevView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

inline bool DevView::IsActive() const noexcept
{
    return m_application->GetApplicationState().views_active.debug;
}

inline void DevView::SetIsActive(bool const open)
{
    m_application->GetApplicationState().views_active.debug = open;
}

inline std::string_view DevView::GetDisplayName() const noexcept
{
    return "Debug";
}

} // namespace Fluxion::Application::Views
