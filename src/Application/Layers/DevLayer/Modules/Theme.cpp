/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Theme.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Implementation of @see Theme.hpp.
///

#include "Graphite/Logger/Logger.hpp"

namespace Fluxion::Application::Layers::Modules::DevLayer {

void SetupDarkTheme()
{
    LOG_SCOPE("");
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    // --- 1. Sizing and Spacing (Modern & Tight) ---
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.CellPadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.ScrollbarSize = 13.0f;
    style.GrabMinSize = 10.0f;

    // --- 2. Borders & Rounding ---
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    style.WindowRounding = 4.0f;
    style.ChildRounding = 3.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
}

void SetupForestGreenTheme()
{
    LOG_SCOPE("");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- 1. Sizing and Spacing ---
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.CellPadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;

    // --- 2. Borders ---
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    // --- 3. Rounding ---
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 4.0f;

    // --- 4. Full Color Palette ---

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.85f, 0.90f, 0.85f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.55f, 0.50f, 1.00f);

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.09f, 0.06f, 1.00f); // Deep pine
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.11f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.10f, 0.07f, 0.96f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.18f, 0.28f, 0.18f, 0.80f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames (Inputs, Checkboxes, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.18f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.30f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.42f, 0.24f, 1.00f);

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.14f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.26f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.08f, 0.05f, 1.00f);

    // Menus
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.14f, 0.09f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.08f, 0.05f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.18f, 0.28f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.38f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.32f, 0.48f, 0.32f, 1.00f);

    // Interactables
    colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.75f, 0.45f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.55f, 0.35f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.70f, 0.45f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.35f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.35f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);

    // Separators and Resizing
    colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.28f, 0.18f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.18f, 0.35f, 0.18f, 0.80f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.22f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.38f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.15f, 0.08f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.22f, 0.12f, 1.00f);

    // Plots
    colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.70f, 0.40f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.50f, 0.85f, 0.50f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.70f, 0.40f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.50f, 0.85f, 0.50f, 1.00f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.12f, 0.22f, 0.12f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.20f, 0.35f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.15f, 0.25f, 0.15f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.08f, 0.14f, 0.08f, 0.50f);

    // Misc
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 0.55f, 0.25f, 0.50f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.60f, 0.90f, 0.60f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.85f, 0.90f, 0.85f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.15f, 0.10f, 0.50f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.08f, 0.05f, 0.60f);

#ifdef IMGUI_HAS_DOCK
    // Docking (If using the docking branch)
    colors[ImGuiCol_DockingPreview] = ImVec4(0.25f, 0.55f, 0.25f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.06f, 0.09f, 0.06f, 1.00f);
#endif
}

void SetupImGuiAmethystTheme()
{
    LOG_SCOPE("");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- 1. Sizing and Spacing (Modern & Tight) ---
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.CellPadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.ScrollbarSize = 13.0f;
    style.GrabMinSize = 10.0f;

    // --- 2. Borders & Rounding ---
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    style.WindowRounding = 4.0f;
    style.ChildRounding = 3.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;

    // --- 3. Color Palette ---

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.90f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.50f, 0.60f, 1.00f);

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.07f, 0.12f, 1.00f); // Deep charcoal-purple
    colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.09f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.07f, 0.12f, 0.96f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.20f, 0.35f, 0.80f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames (Inputs, Checkboxes, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.12f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.20f, 0.38f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.25f, 0.55f, 1.00f);

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.09f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.14f, 0.32f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.05f, 0.10f, 1.00f);

    // Menus
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.09f, 0.18f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.07f, 0.05f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.20f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.30f, 0.50f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.40f, 0.65f, 1.00f);

    // Interactables (The "Pop" colors)
    colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.45f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.35f, 0.75f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65f, 0.45f, 0.95f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.20f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.28f, 0.62f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.35f, 0.80f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.20f, 0.40f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.28f, 0.62f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.50f, 0.35f, 0.80f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.12f, 0.25f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.28f, 0.62f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.20f, 0.45f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.08f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.12f, 0.25f, 1.00f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.15f, 0.28f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.20f, 0.40f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.15f, 0.30f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);

    // Misc
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.50f, 0.35f, 0.80f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.80f, 0.65f, 1.00f, 0.95f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.45f, 0.90f, 1.00f);

#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.50f, 0.35f, 0.80f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.09f, 0.07f, 0.12f, 1.00f);
#endif
}

void SetupImGuiSapphireTheme()
{
    LOG_SCOPE("");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- 1. Sizing and Spacing ---
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ScrollbarSize = 15.0f;
    style.GrabMinSize = 10.0f;

    // --- 2. Borders & Rounding ---
    style.WindowRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    // --- 3. Color Palette ---

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.93f, 0.97f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.50f, 0.65f, 1.00f);

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.09f, 0.12f, 1.00f); // Deep midnight
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.09f, 0.12f, 0.95f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.25f, 0.35f, 0.70f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames (Inputs, Checkboxes, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.18f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.28f, 0.40f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.38f, 0.55f, 1.00f);

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.22f, 0.35f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.08f, 0.12f, 1.00f);

    // Menus
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.16f, 0.22f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.08f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.32f, 0.48f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.42f, 0.60f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.50f, 0.75f, 1.00f);

    // Interactables
    colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.55f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.48f, 0.75f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.48f, 0.75f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.60f, 0.90f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.20f, 0.32f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.45f, 0.70f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.12f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.20f, 0.32f, 1.00f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15f, 0.25f, 0.40f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.20f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.15f, 0.25f, 0.40f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);

    // Misc
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.55f, 0.85f, 0.40f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.50f, 0.80f, 1.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);

#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.25f, 0.50f, 0.80f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.07f, 0.09f, 0.12f, 1.00f);
#endif
}

void SetupImGuiAmberYellowTheme()
{
    LOG_SCOPE("");
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- 1. Sizing and Spacing (Sharp & Technical) ---
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.CellPadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    // --- 2. Borders & Rounding (Low rounding for a technical look) ---
    style.WindowRounding = 2.0f;
    style.ChildRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 2.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    // --- 3. Color Palette ---

    // Text
    colors[ImGuiCol_Text] = ImVec4(1.00f, 0.95f, 0.80f, 1.00f); // Soft cream-yellow
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.45f, 0.30f, 1.00f);

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.06f, 1.00f); // Near black
    colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.09f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.06f, 0.96f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.25f, 0.10f, 0.80f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames (Inputs, Checkboxes, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.14f, 0.10f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.22f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.30f, 0.15f, 1.00f);

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.11f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.18f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.04f, 1.00f);

    // Menus
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.11f, 0.08f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.04f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.30f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.40f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.50f, 0.20f, 1.00f);

    // Interactables (The High-Vis Amber)
    colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.80f, 0.10f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.60f, 0.10f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.95f, 0.80f, 0.10f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.30f, 0.25f, 0.05f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.38f, 0.10f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.50f, 0.15f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.30f, 0.25f, 0.05f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.38f, 0.10f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.50f, 0.15f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.14f, 0.10f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.38f, 0.10f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.30f, 0.10f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.07f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.14f, 0.10f, 1.00f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.16f, 0.10f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.35f, 0.30f, 0.15f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.20f, 0.10f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

    // Misc
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.95f, 0.80f, 0.10f, 0.25f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.85f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.95f, 0.80f, 0.10f, 1.00f);

#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.95f, 0.80f, 0.10f, 0.40f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.07f, 0.07f, 0.06f, 1.00f);
#endif
}

enum class ETheme : std::uint8_t
{
    Dark,
    ForestGreen,
    Amethyst,
    Sapphire,
    AmberYellow
};

void RenderTheme()
{
    LOG_SCOPE("");
    static auto currentTheme = ETheme::Dark;

    static std::size_t constexpr c_themes_count{5};
    const char* themeNames[c_themes_count] = {
        "Dark", "Forest Green", "Amethyst", "Sapphire", "AmberYellow"};

    int currentIndex = static_cast<int>(currentTheme);
    if (ImGui::Combo("Select Theme", &currentIndex, themeNames, c_themes_count))
    {
        currentTheme = static_cast<ETheme>(currentIndex);

        switch (currentTheme)
        {
        case ETheme::ForestGreen:
            SetupForestGreenTheme();
            break;
        case ETheme::Amethyst:
            SetupImGuiAmethystTheme();
            break;
        case ETheme::Sapphire:
            SetupImGuiSapphireTheme();
            break;
        case ETheme::AmberYellow:
            SetupImGuiAmberYellowTheme();
            break;
        default:
            SetupDarkTheme();
            break;
        }
    }
}

} // namespace Fluxion::Application::Layers::Modules::DevLayer
