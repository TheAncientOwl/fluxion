/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsProgressView.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Main view responsible for rendering import progress.
///

#include "LogsProgressView.hpp"
#include "Fluxion/Common/Utility/Math.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::LogsProgressView);
USE_LOG_SCOPE(Fluxion::Application::Views::LogsProgressView);

namespace Fluxion::Application::Views {

std::string_view LogsProgressView::GetViewName() noexcept
{
    return "LogsProgressView";
}

std::string_view LogsProgressView::GetName() const noexcept
{
    return LogsProgressView::GetViewName();
}

LogsProgressView::LogsProgressView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TView{std::move(application), render_priority}
{
    LOG_SCOPE("::LogsProgressView()");
}

void LogsProgressView::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
}

void LogsProgressView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void LogsProgressView::OnRender()
{
    LOG_SCOPE("::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    if (app_state.logs_progress.operation == Fluxion::Application::ELogsOperation::None)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(600.0f, 100.0f));
    ImGui::Begin(ICON_CI_OUTPUT " Logs Progress", nullptr, ImGuiWindowFlags_NoResize);

    auto render_progress = [&](const char* icon, const char* action_name, std::size_t total) {
        auto const processed{app_state.logs_plugin->GetProcessedLogsProgress()};
        auto const percentage{Fluxion::Common::Utility::Math::Percentage(processed, total)};

        ImGui::Text("%s %s %zu/%zu logs", icon, action_name, processed, total);
        Graphite::Common::UI::ProgressBar(percentage);
    };

    switch (app_state.logs_progress.operation)
    {
    case Fluxion::Application::ELogsOperation::Import:
        render_progress(
            ICON_CI_ROCKET, "Imported", app_state.logs_plugin->GetTotalEstimatedImportLogs());
        break;

    case Fluxion::Application::ELogsOperation::Filter:
        render_progress(ICON_CI_WAND, "Filtered", app_state.logs_plugin->GetTotalLogs());
        break;

    case Fluxion::Application::ELogsOperation::DisableFilter:
        render_progress(ICON_CI_WAND, "Removed filters", app_state.logs_plugin->GetTotalLogs());
        break;

    case Fluxion::Application::ELogsOperation::Search:
        render_progress(ICON_CI_SEARCH, "Searched", app_state.logs_plugin->GetTotalLogs());
        break;

    default:
        LOG_WARN(
            "Not handled Fluxion::Application::ELogsOperation::{}",
            static_cast<std::uint8_t>(app_state.logs_progress.operation));
        break;
    }

    ImGui::End();
}

inline bool LogsProgressView::IsActive() const noexcept
{
    return m_application->GetApplicationState().logs_progress.operation !=
           Fluxion::Application::ELogsOperation::None;
}

inline void LogsProgressView::SetIsActive(bool const /*open*/)
{
    // Do nothing... Active state is handled via @see AppState::logs_progress::operation
}

void LogsProgressView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

} // namespace Fluxion::Application::Views
