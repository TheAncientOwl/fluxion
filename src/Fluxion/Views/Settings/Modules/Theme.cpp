/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Theme.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Implementation of @see Theme.hpp.
///

#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::Modules::SettingsView::Themes);
USE_LOG_SCOPE(Fluxion::Application::Views::Modules::SettingsView::Themes);

namespace Fluxion::Application::Views::Modules::SettingsView {

#include <imgui.h>

// Shared compact metrics helper for consistent, space-efficient sizing across themes
static void ApplyCompactModernSizing(ImGuiStyle& style, float rounding = 4.0f, bool hasFrameBorder = false)
{
    // Padding & Spacing
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    // Borders & Rounding
    style.WindowRounding = rounding;
    style.ChildRounding = rounding;
    style.FrameRounding = rounding * 0.75f;
    style.PopupRounding = rounding * 0.75f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = rounding * 0.75f;
    style.TabRounding = rounding * 0.75f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = hasFrameBorder ? 1.0f : 0.0f;
    style.PopupBorderSize = 1.0f;
}

// -----------------------------------------------------------------------------
// 1. Dark (Modern Neutral Zinc/Slate)
// -----------------------------------------------------------------------------
void SetupImGuiDarkStyle()
{
    LOG_SCOPE("::SetupImGuiDarkStyle()");
    LOG_INFO("::SetupImGuiDarkStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();

    ApplyCompactModernSizing(style, 4.0f, true);

    ImGui::StyleColorsDark();
}

// -----------------------------------------------------------------------------
// 2. Forest Green (Deep Pines & Emerald Accent)
// -----------------------------------------------------------------------------
void SetupForestGreenStyle()
{
    LOG_SCOPE("::SetupForestGreenStyle()");
    LOG_INFO("::SetupForestGreenStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 4.0f, false);

    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.94f, 0.91f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.50f, 0.44f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.12f, 0.09f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.09f, 0.07f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.14f, 0.10f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.24f, 0.17f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.28f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.09f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.12f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.09f, 0.07f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.10f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.09f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.15f, 0.24f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.28f, 0.44f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.31f, 0.85f, 0.48f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.31f, 0.85f, 0.48f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.42f, 0.95f, 0.59f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.13f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.45f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.38f, 0.22f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.13f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.28f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.35f, 0.25f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.12f, 0.09f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.28f, 0.20f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.13f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.11f, 0.17f, 0.12f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.15f, 0.24f, 0.17f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.12f, 0.18f, 0.13f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.31f, 0.85f, 0.48f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.45f, 0.27f, 0.50f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.31f, 0.85f, 0.48f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.31f, 0.85f, 0.48f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.06f, 0.09f, 0.07f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 3. Amethyst (Deep Violet & Royal Purple)
// -----------------------------------------------------------------------------
void SetupImGuiAmethystStyle()
{
    LOG_SCOPE("::SetupImGuiAmethystStyle()");
    LOG_INFO("::SetupImGuiAmethystStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 5.0f, false);

    colors[ImGuiCol_Text] = ImVec4(0.93f, 0.91f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.43f, 0.58f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.08f, 0.14f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.06f, 0.11f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.09f, 0.17f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.22f, 0.17f, 0.31f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.14f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.19f, 0.37f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.24f, 0.47f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.06f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.08f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.06f, 0.11f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.07f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.06f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.17f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.32f, 0.24f, 0.47f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.41f, 0.30f, 0.60f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.67f, 0.44f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.67f, 0.44f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.77f, 0.58f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.14f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.23f, 0.76f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.38f, 0.18f, 0.64f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.14f, 0.26f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.19f, 0.37f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.24f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.08f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.19f, 0.37f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.14f, 0.26f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.14f, 0.11f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.22f, 0.17f, 0.31f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.17f, 0.13f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.67f, 0.44f, 0.98f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.46f, 0.23f, 0.76f, 0.50f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.67f, 0.44f, 0.98f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.67f, 0.44f, 0.98f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.06f, 0.11f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 4. Sapphire (Midnight Navy & Electric Blue)
// -----------------------------------------------------------------------------
void SetupImGuiSapphireStyle()
{
    LOG_SCOPE("::SetupImGuiSapphireStyle()");
    LOG_INFO("::SetupImGuiSapphireStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 4.0f, false);

    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.93f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.48f, 0.58f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.10f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.12f, 0.18f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.14f, 0.20f, 0.31f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.17f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.24f, 0.36f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.21f, 0.30f, 0.45f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.10f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.08f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.14f, 0.20f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.21f, 0.30f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.27f, 0.39f, 0.58f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.23f, 0.51f, 0.96f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.23f, 0.51f, 0.96f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.38f, 0.65f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.17f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.38f, 0.75f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.11f, 0.30f, 0.61f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.12f, 0.17f, 0.26f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.24f, 0.36f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.21f, 0.30f, 0.45f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.10f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.17f, 0.24f, 0.36f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.17f, 0.26f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.09f, 0.13f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.14f, 0.20f, 0.31f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.11f, 0.16f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.23f, 0.51f, 0.96f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15f, 0.38f, 0.75f, 0.50f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.23f, 0.51f, 0.96f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.23f, 0.51f, 0.96f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.05f, 0.07f, 0.11f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 5. Amber Yellow (Warm Slate & Golden Amber)
// -----------------------------------------------------------------------------
void SetupImGuiAmberYellowStyle()
{
    LOG_SCOPE("::SetupImGuiAmberYellowStyle()");
    LOG_INFO("::SetupImGuiAmberYellowStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 4.0f, true);

    colors[ImGuiCol_Text] = ImVec4(0.96f, 0.93f, 0.86f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.53f, 0.49f, 0.42f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.11f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.08f, 0.07f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.13f, 0.11f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.22f, 0.18f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.19f, 0.17f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.23f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.34f, 0.29f, 0.21f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.08f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.11f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.08f, 0.07f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.08f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.22f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.34f, 0.29f, 0.21f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.44f, 0.37f, 0.26f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.96f, 0.62f, 0.04f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.96f, 0.62f, 0.04f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.73f, 0.20f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.19f, 0.17f, 0.14f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.72f, 0.44f, 0.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.58f, 0.35f, 0.00f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.19f, 0.17f, 0.14f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.27f, 0.23f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.34f, 0.29f, 0.21f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.11f, 0.10f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.23f, 0.18f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.17f, 0.14f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.16f, 0.14f, 0.12f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.22f, 0.18f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.18f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.96f, 0.62f, 0.04f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.72f, 0.44f, 0.00f, 0.50f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.96f, 0.62f, 0.04f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.96f, 0.62f, 0.04f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.09f, 0.08f, 0.07f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 6. Dracula (Official Dracula Dark Palette - Compact)
// -----------------------------------------------------------------------------
void SetupImGuiDraculaStyle()
{
    LOG_SCOPE("::SetupImGuiDraculaStyle()");
    LOG_INFO("::SetupImGuiDraculaStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 4.0f, true);

    colors[ImGuiCol_Text] = ImVec4(0.97f, 0.97f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.21f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.21f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.31f, 0.98f, 0.48f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.74f, 0.58f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.84f, 0.68f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.47f, 0.78f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.37f, 0.62f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.55f, 0.91f, 0.99f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.74f, 0.58f, 0.98f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.74f, 0.58f, 0.98f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 7. Catppuccin Mocha (Soft Dark Pastels - Compact)
// -----------------------------------------------------------------------------
void SetupImGuiCatppuccinMochaStyle()
{
    LOG_SCOPE("::SetupImGuiCatppuccinMochaStyle()");
    LOG_INFO("::SetupImGuiCatppuccinMochaStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 5.0f, false);

    colors[ImGuiCol_Text] = ImVec4(0.80f, 0.84f, 0.96f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.45f, 0.55f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.11f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.26f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.37f, 0.38f, 0.51f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.42f, 0.45f, 0.55f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.78f, 0.93f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.78f, 0.93f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.65f, 0.97f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.55f, 0.87f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.26f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.26f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.19f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.94f, 0.72f, 0.42f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.31f, 0.32f, 0.42f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.71f, 0.75f, 1.00f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.71f, 0.75f, 1.00f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 8. Gruvbox Hard (Industrial Retro Contrast - Compact)
// -----------------------------------------------------------------------------
void SetupImGuiGruvboxHardStyle()
{
    LOG_SCOPE("::SetupImGuiGruvboxHardStyle()");
    LOG_INFO("::SetupImGuiGruvboxHardStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 2.0f, true);

    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.86f, 0.70f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.57f, 0.51f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.13f, 0.13f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.13f, 0.13f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.14f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.57f, 0.51f, 0.45f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.72f, 0.73f, 0.15f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.65f, 0.60f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.73f, 0.67f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.29f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.36f, 0.33f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.24f, 0.22f, 0.21f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.98f, 0.74f, 0.18f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.31f, 0.29f, 0.27f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.29f, 0.20f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.72f, 0.73f, 0.15f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.11f, 0.13f, 0.13f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 9. Crimson Vesuvius (Deep Basalt Charcoal & Fiery Crimson)
// -----------------------------------------------------------------------------
void SetupImGuiCrimsonVesuviusStyle()
{
    LOG_SCOPE("::SetupImGuiCrimsonVesuviusStyle()");
    LOG_INFO("::SetupImGuiCrimsonVesuviusStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 4.0f, true);

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.10f, 0.10f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.26f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.29f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.38f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.26f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.88f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.35f, 0.28f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.72f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.29f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.17f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.26f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.21f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.98f, 0.35f, 0.28f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.72f, 0.18f, 0.18f, 0.50f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.88f, 0.22f, 0.22f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.88f, 0.22f, 0.22f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.09f, 0.07f, 0.07f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 10. Rose Quartz (Moody Charcoal & Soft Blush Accent)
// -----------------------------------------------------------------------------
void SetupImGuiRoseQuartzStyle()
{
    LOG_SCOPE("::SetupImGuiRoseQuartzStyle()");
    LOG_INFO("::SetupImGuiRoseQuartzStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 6.0f, false);

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.91f, 0.93f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.54f, 0.46f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.11f, 0.13f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.19f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.46f, 0.32f, 0.39f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.55f, 0.68f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.95f, 0.55f, 0.68f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.99f, 0.68f, 0.78f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.19f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.42f, 0.56f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.33f, 0.45f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.19f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.27f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.16f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.19f, 0.22f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.95f, 0.55f, 0.68f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.85f, 0.42f, 0.56f, 0.50f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.95f, 0.55f, 0.68f, 1.00f);
#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.95f, 0.55f, 0.68f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.09f, 0.08f, 0.09f, 1.00f);
#endif
}

// -----------------------------------------------------------------------------
// 11. Cyberpunk (Pitch Black, High-Contrast Neon Yellow & Cyan)
// -----------------------------------------------------------------------------
void SetupImGuiCyberpunkStyle()
{
    LOG_SCOPE("::SetupImGuiCyberpunkStyle()");
    LOG_INFO("::SetupImGuiCyberpunkStyle(): setting theme");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    ImGui::StyleColorsDark();

    ApplyCompactModernSizing(style, 0.0f, true); // Sharp 0.0f edges for hard-surface sci-fi look

    // Main Text & Backgrounds
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.96f, 0.98f, 1.00f); // High-contrast crisp cyan-white
    colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.40f, 0.48f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.07f, 1.00f); // Void midnight
    colors[ImGuiCol_ChildBg] = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.09f, 0.98f);

    // Glowing Neon Borders
    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.94f, 1.00f, 0.40f); // Electric Cyan (subtle idle glow)
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame Controls (Inputs, Checkboxes, Combo Boxes)
    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.00f, 0.45f, 0.35f); // Neon Magenta glow
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.94f, 1.00f, 0.35f); // Electric Cyan glow

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.03f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.35f, 0.02f, 0.18f, 1.00f); // Deep Magenta accent
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.02f, 0.02f, 0.04f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.05f, 0.05f, 0.08f, 1.00f);

    // Scrollbars & Sliders
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.94f, 1.00f, 0.60f); // Electric Cyan grab
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.95f, 0.00f, 0.45f, 0.80f); // Hot Pink hover
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.98f, 0.90f, 0.00f, 1.00f); // Warning Yellow active
    colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.94f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.98f, 0.90f, 0.00f, 1.00f); // Electric Yellow
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

    // Buttons & Interactive Elements
    colors[ImGuiCol_Button] = ImVec4(0.09f, 0.11f, 0.16f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.95f, 0.00f, 0.45f, 1.00f); // Hot Magenta hover
    colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.94f, 1.00f, 1.00f); // Electric Cyan active

    // Headers & Tree Nodes
    colors[ImGuiCol_Header] = ImVec4(0.12f, 0.14f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.95f, 0.00f, 0.45f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.94f, 1.00f, 0.80f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.00f, 0.45f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.04f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.03f, 0.03f, 0.05f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.09f, 0.14f, 1.00f);

    // Tables & Data Grids
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.08f, 0.09f, 0.15f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] =
        ImVec4(0.00f, 0.94f, 1.00f, 0.50f); // Electric Cyan outer grid
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.00f, 0.94f, 1.00f, 0.20f); // Subtle inner grid lines
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.00f, 0.94f, 1.00f, 0.03f); // Faint cyan alternating tint

    // Selections & Highlights
    colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.94f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.95f, 0.00f, 0.45f, 0.40f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.94f, 1.00f, 1.00f);

#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.95f, 0.00f, 0.45f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.02f, 0.02f, 0.04f, 1.00f);
#endif
}

enum class ETheme : std::uint8_t
{
    Dark,
    ForestGreen,
    Cyberpunk,
    Amethyst,
    Sapphire,
    AmberYellow,
    Dracula,
    CatppuccinMocha,
    GruvboxHard,
    CrimsonVesuvius,
    RoseQuartz,
};

void RenderTheme()
{
    LOG_SCOPE("::RenderTheme()");
    static auto currentTheme{ETheme::Dark};

    static std::size_t constexpr c_themes_count{11};
    const char* themeNames[c_themes_count] = {
        "Dark",
        "Forest Green",
        "Cyberpunk",
        "Amethyst",
        "Sapphire",
        "AmberYellow",
        "Dracula",
        "CatppuccinMocha",
        "GruvboxHard",
        "CrimsonVesuvius",
        "RoseQuartz"};

    int currentIndex = static_cast<int>(currentTheme);
    if (ImGui::Combo("Select Theme", &currentIndex, themeNames, c_themes_count))
    {
        currentTheme = static_cast<ETheme>(currentIndex);

        switch (currentTheme)
        {
        case ETheme::ForestGreen:
            SetupForestGreenStyle();
            break;
        case ETheme::Amethyst:
            SetupImGuiAmethystStyle();
            break;
        case ETheme::Sapphire:
            SetupImGuiSapphireStyle();
            break;
        case ETheme::AmberYellow:
            SetupImGuiAmberYellowStyle();
            break;
        case ETheme::Dracula:
            SetupImGuiDraculaStyle();
            break;
        case ETheme::CatppuccinMocha:
            SetupImGuiCatppuccinMochaStyle();
            break;
        case ETheme::GruvboxHard:
            SetupImGuiGruvboxHardStyle();
            break;
        case ETheme::CrimsonVesuvius:
            SetupImGuiCrimsonVesuviusStyle();
            break;
        case ETheme::RoseQuartz:
            SetupImGuiRoseQuartzStyle();
            break;
        case ETheme::Cyberpunk:
            SetupImGuiCyberpunkStyle();
            break;
        default:
            SetupImGuiDarkStyle();
            break;
        }
    }
}

} // namespace Fluxion::Application::Views::Modules::SettingsView
