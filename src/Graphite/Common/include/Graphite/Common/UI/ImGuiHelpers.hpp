/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImGuiHelpers.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Wrappers for ImGui UI elements.
///

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace Graphite::Common::UI {

template <typename... Args>
void ItemHoverTooltip(const char* fmt, Args&&... args)
{
    if (ImGui::IsItemHovered())
    {
        if constexpr (sizeof...(args) == 0)
        {
            ImGui::SetTooltip("%s", fmt);
        }
        else
        {
            ImGui::SetTooltip(fmt, std::forward<Args>(args)...);
        }
    }
}

template <typename TAction>
inline void IconButton(const char* icon, const char* tooltip, TAction&& action)
{
    if (ImGui::Button(icon))
    {
        action();
    }
    ItemHoverTooltip(tooltip);
}

enum class EInputTextWidth : std::uint8_t
{
    Auto,
    Fill
};

template <EInputTextWidth TInputTextWidth = EInputTextWidth::Fill>
bool InputText(const char* label, std::string& str)
{
    bool modified{false};

    if constexpr (TInputTextWidth == EInputTextWidth::Fill)
    {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    }

    if (ImGui::InputText(label, &str))
    {
        modified = true;
    }

    if constexpr (TInputTextWidth == EInputTextWidth::Fill)
    {
        ImGui::PopItemWidth();
    }

    return modified;
}

void VerticalSeparator(float height = 0.0f, float thickness = 1.0f, float reserved_width = 5.0f);

}; // namespace Graphite::Common::UI
