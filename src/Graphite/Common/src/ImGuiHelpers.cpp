/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImGuiHelpers.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Implementation of @see Graphite/Common/UI/ImGuiHelpers.hpp
///

#include <algorithm>

#include "Graphite/Common/UI/ImGuiHelpers.hpp"

namespace Graphite::Common::UI {

void VerticalSeparator(float height, float thickness, float reserved_width)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float frame_height = ImGui::GetFrameHeight();

    // height of the line
    if (height <= 0.0f)
        height = frame_height * 0.7f;

    ImVec2 pos = ImGui::GetCursorScreenPos();

    // horizontal center inside reserved space
    float x_center = pos.x + reserved_width * 0.5f;
    float y_start = pos.y + (frame_height - height) * 0.5f;

    ImU32 color = ImGui::GetColorU32(ImGuiCol_Separator);

    draw_list->AddLine(ImVec2(x_center, y_start), ImVec2(x_center, y_start + height), color, thickness);

    // reserve the width for layout
    ImGui::Dummy(ImVec2(reserved_width, frame_height));
    ImGui::SameLine();
}

void ProgressBar(float const current_percent, float const width, const char* const overlay)
{
    auto const percent = std::clamp(current_percent, 0.0f, 100.0f);
    auto const fraction = percent / 100.0f;

    char default_overlay_buf[32];
    if (!overlay)
    {
        std::snprintf(
            default_overlay_buf, sizeof(default_overlay_buf), "%.1f%%", static_cast<double>(percent));
    }
    const char* const text_to_render = overlay ? overlay : default_overlay_buf;

    // 1. Render progress bar frame
    ImGui::ProgressBar(fraction, ImVec2(width, 0.0f), "");

    // 2. Get bounding box
    ImVec2 const min = ImGui::GetItemRectMin();
    ImVec2 const max = ImGui::GetItemRectMax();

    // 3. Center overlay text
    ImVec2 const text_size = ImGui::CalcTextSize(text_to_render);
    ImVec2 const text_pos = ImVec2(
        min.x + (max.x - min.x - text_size.x) * 0.5f, min.y + (max.y - min.y - text_size.y) * 0.5f);

    auto* draw_list = ImGui::GetWindowDrawList();

    // 4. Subtle 1px drop-shadow (keeps icon/font clean without heavy border distortion)
    draw_list->AddText(
        ImVec2(text_pos.x + 1.0f, text_pos.y + 1.0f), IM_COL32(0, 0, 0, 200), text_to_render);

    // 5. Crisp primary text
    draw_list->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), text_to_render);
}

} // namespace Graphite::Common::UI
