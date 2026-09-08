/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AboutView.cpp
/// @author Alexandru Delegeanu
/// @version 0.12
/// @brief Implementation of @see AboutView.hpp.
///

#include "AboutView.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "IconsCodicons.h"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::AboutView);
USE_LOG_SCOPE(Fluxion::Application::Views::AboutView);

namespace Fluxion::Application::Views {

std::string_view AboutView::GetViewName() noexcept
{
    return "AboutView";
}

std::string_view AboutView::GetName() const noexcept
{
    return AboutView::GetViewName();
}

AboutView::AboutView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TSoftCloseableView{std::move(application), render_priority}
{
    LOG_SCOPE("::AboutView()");
}

void AboutView::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
}

void AboutView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void AboutView::OnRender()
{
    LOG_SCOPE("::OnRender()");

    if (ImGui::Begin(ICON_CI_TELESCOPE " About"))
    {
        // App Title & Version Header
        ImGui::Text("Version");

        ImGui::SameLine();
        ImGui::TextDisabled("%s", FLUXION_VERSION);

        ImGui::Separator();
        ImGui::Spacing();

        // Description
        ImGui::TextWrapped(
            "Lightning-fast, high-performance cross-platform log viewer built to effortlessly "
            "handle massive log files without breaking a sweat.");

        ImGui::Spacing();
        ImGui::Spacing();

        // Tech Stack / Details
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Built With:");
        ImGui::BulletText(
            "C++23 – Core application and high-performance plugins engine for parsing logs");
        ImGui::BulletText("Graphite – Custom APP framework that uses ImGui for rendering");
        ImGui::BulletText("Google RE2 – Thread-safe, high-performance regular expression matching");
        ImGui::BulletText(
            "Google Test (gtest) – Comprehensive unit and integration testing framework");
        ImGui::BulletText("CMake – Cross-platform build configuration system");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Links Section
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Resources & Support:");

        ImGui::SameLine();
        Graphite::Common::UI::Hyperlink(
            "GitHub Repository", "https://github.com/TheAncientOwl/fluxion");

        ImGui::SameLine();
        ImGui::TextDisabled(" | ");
        ImGui::SameLine();

        Graphite::Common::UI::Hyperlink(
            "Report an Issue", "https://github.com/TheAncientOwl/fluxion/issues");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // License Section
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "License:");

        ImGui::SameLine();
        Graphite::Common::UI::Hyperlink(
            "MIT", "https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE");

        ImGui::SameLine();
        ImGui::TextDisabled(" | ");
        ImGui::SameLine();

        Graphite::Common::UI::Hyperlink(
            "ThirdParty",
            "https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE-THIRD-PARTY.md");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Copyright / Footer
        ImGui::TextDisabled("Copyright (c) 2026 Alexandru Delegeanu.");
    }

    ImGui::End();
}

void AboutView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

inline bool AboutView::IsActive() const noexcept
{
    return m_application->GetApplicationState().views_active.about;
}

inline void AboutView::SetIsActive(bool const open)
{
    m_application->GetApplicationState().views_active.about = open;
}

inline std::string_view AboutView::GetDisplayName() const noexcept
{
    return "About";
}

} // namespace Fluxion::Application::Views
