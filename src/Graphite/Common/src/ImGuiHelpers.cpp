/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImGuiHelpers.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
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

void ProgressBar(float const current_percent)
{
    auto const percent = std::clamp(current_percent, 0.0f, 100.0f);
    auto const fraction = percent / 100.0f;

    char overlay_buf[32];
    std::snprintf(overlay_buf, sizeof(overlay_buf), "%.1f%%", static_cast<double>(percent));

    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay_buf);
}

} // namespace Graphite::Common::UI
