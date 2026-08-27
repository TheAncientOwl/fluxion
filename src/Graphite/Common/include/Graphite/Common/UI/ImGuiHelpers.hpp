/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImGuiHelpers.hpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Wrappers for ImGui UI elements.
///

#include <format>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace Graphite::Common::UI {

inline void ItemHoverTooltip(const char* text)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", text);
    }
}

template <typename... Args>
void ItemHoverTooltip(std::format_string<std::remove_cvref_t<Args>...> fmt, Args&&... args)
{
    if (ImGui::IsItemHovered())
    {
        auto const formatted = std::vformat(fmt.get(), std::make_format_args(args...));
        ImGui::SetTooltip("%s", formatted.c_str());
    }
}

template <typename TAction>
inline bool IconButton(const char* icon, const char* tooltip, TAction&& action)
{
    bool clicked{false};
    if (ImGui::Button(icon))
    {
        clicked = true;
        action();
    }
    ItemHoverTooltip(tooltip);
    return clicked;
}

template <typename TAction>
inline bool TabItemIconButton(const char* icon, const char* tooltip, TAction&& action)
{
    bool clicked{false};
    if (ImGui::TabItemButton(icon))
    {
        clicked = true;
        action();
    }
    ItemHoverTooltip(tooltip);
    return clicked;
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

void ProgressBar(float const current_percent);

}; // namespace Graphite::Common::UI
