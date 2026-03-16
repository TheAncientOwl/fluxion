/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Implementation of @see FiltersLayer.hpp.
///

#include "icons/IconsCodicons.h"
#include "imgui/imgui.h"

#include "FiltersLayer.hpp"

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
        m_application->RemoveLayer(m_layer_id);
    }
}

void FiltersLayer::OnRemove()
{
    LOG_SCOPE("");
}

void VerticalSeparator(float height = 0.0f, float thickness = 1.0f, float reserved_width = 5.0f)
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

void ColorsPicker(
    const char* id,
    Fluxion::API::Data::FilterColors& colors,
    ImVec4 const& display,
    std::string_view const preview)
{
    static bool s_editing_fg = true;

    if (ImGui::ColorButton(id, display))
    {
        ImGui::OpenPopup(id);
    }

    if (ImGui::BeginPopup(id))
    {
        // --- Checkbox to switch FG/BG ---
        // ImGui::Checkbox("Edit Foreground", &s_editing_fg);
        if (ImGui::ColorButton(
                "FG", colors.foreground, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview))
        {
            s_editing_fg = true;
        }
        ImGui::SameLine();
        if (ImGui::ColorButton(
                "BG", colors.background, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview))
        {
            s_editing_fg = false;
        }

        // --- Preview ---
        ImGui::SameLine();
        auto const padding = ImVec2(5.0f, 3.0f);
        auto const cursor_pos = ImGui::GetCursorScreenPos();
        auto const rect_max = ImVec2(
            cursor_pos.x + ImGui::GetContentRegionAvail().x,
            cursor_pos.y + ImGui::GetTextLineHeight() + 2.0f * padding.y);

        ImGui::GetWindowDrawList()->AddRectFilled(
            cursor_pos, rect_max, ImGui::GetColorU32(colors.background), 4.0f);

        ImGui::SetCursorScreenPos(ImVec2(cursor_pos.x + padding.x, cursor_pos.y + padding.y));
        ImGui::TextColored(colors.foreground, "%s", preview.data());

        ImGui::Separator();

        auto& target = s_editing_fg ? colors.foreground : colors.background;

        auto const brightness =
            0.299f * target.x + 0.587f * target.y + 0.114f * target.z; // perceived luminance
        auto const grab_color = brightness < 0.5f ? ImVec4(
                                                        std::min(target.x + 0.6f, 1.0f),
                                                        std::min(target.y + 0.6f, 1.0f),
                                                        std::min(target.z + 0.6f, 1.0f),
                                                        1.0f)
                                                  : ImVec4(
                                                        std::max(target.x - 0.6f, 0.0f),
                                                        std::max(target.y - 0.6f, 0.0f),
                                                        std::max(target.z - 0.6f, 0.0f),
                                                        1.0f);
        auto const text_color = (target.x + target.y + target.z) / 3.0f < 0.5f ? ImVec4(1, 1, 1, 1)
                                                                               : ImVec4(0, 0, 0, 1);

        ImGui::PushStyleColor(ImGuiCol_Text, text_color);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, target);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, target);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, target);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, grab_color);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, grab_color);
        ImGui::SliderFloat("##opacity", &target.w, 0.0f, 1.0f, "%.2f");
        ImGui::PopStyleColor(6);

        ImGui::ColorPicker4("##color", (float*)&target, ImGuiColorEditFlags_NoSidePreview);

        ImGui::EndPopup();
    }
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
        auto PushButtonGrayIfOff = [](bool const state) {
            if (!state)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, {0.5f, 0.5f, 0.5f, 0.6f});
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.6f, 0.6f, 0.6f, 0.9f});
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.4f, 0.4f, 0.4f, 0.3f});
            }
        };

        auto PopButtonGrayIfOff = [](bool const state) {
            if (!state)
            {
                ImGui::PopStyleColor(3);
            }
        };

        auto& filters{app_state.filters.data};
        for (auto& tab : filters.tabs)
        {
            if (ImGui::BeginTabItem(tab.name.c_str()))
            {
                int filter_render_index{0};
                for (auto& filter : filters.filters)
                {
                    if (auto const filter_it = std::find_if(
                            tab.filter_ids.cbegin(),
                            tab.filter_ids.cend(),
                            [&target_id = filter.id](auto const& filter_id) {
                                return target_id == filter_id;
                            });
                        filter_it == tab.filter_ids.cend())
                    {
                        continue;
                    }

                    LOG_TRACE("Tab {} | Filter {}", tab.name, filter.name);

                    auto const filter_child_id{std::to_string(++filter_render_index)};
                    ImGui::BeginChild(
                        filter_child_id.c_str(), ImVec2{0, 0}, ImGuiChildFlags_AutoResizeY);
                    ImGui::Separator();

                    using EFilterFlag = Fluxion::API::Data::EFilterFlag;

                    if (ImGui::Button(
                            filter[EFilterFlag::IsCollapsed] ? ICON_CI_EYE_CLOSED : ICON_CI_EYE))
                    {
                        // TODO: implement collapse
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip(
                            filter[EFilterFlag::IsCollapsed] ? "Show Filter" : "Hide Filter");
                    }

                    ImGui::SameLine();
                    if (ImGui::Button(ICON_CI_COPY))
                    {
                        // TODO: implement duplicate
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Duplicate Filter");
                    }

                    ImGui::PushStyleColor(ImGuiCol_Button, {0.75f, 0.0f, 0.0f, 0.6f});
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.75f, 0.0f, 0.0f, 0.9f});
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.75f, 0.0f, 0.0f, 0.3f});
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_CI_TRASH))
                    {
                        // TODO: implement delete
                    }
                    ImGui::PopStyleColor(3);
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Delete Filter");
                    }

                    ImGui::SameLine();
                    ColorsPicker(
                        "##FG",
                        filter.colors,
                        filter.colors.foreground,
                        "Lorem ipsum dolor sit amet");
                    ImGui::SameLine();
                    ColorsPicker(
                        "##BG",
                        filter.colors,
                        filter.colors.background,
                        "Lorem ipsum dolor sit amet");

                    ImGui::SameLine();
                    VerticalSeparator();

                    ImGui::SameLine();
                    int priority_tmp = static_cast<int>(filter.priority);
                    ImGui::PushItemWidth(85);
                    if (ImGui::InputInt("##PR", &priority_tmp, 1, 1))
                    {
                        priority_tmp = std::clamp(priority_tmp, 0, 255);
                        filter.priority = static_cast<std::uint8_t>(priority_tmp);
                    }
                    ImGui::PopItemWidth();
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Filter Priority");
                    }

                    ImGui::SameLine();
                    VerticalSeparator();

                    char name_buf[128];
                    std::strncpy(name_buf, filter.name.c_str(), sizeof(name_buf));
                    name_buf[sizeof(name_buf) - 1] = '\0';
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, filter.colors.foreground);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, filter.colors.background);
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                    if (ImGui::InputText("##FN", name_buf, sizeof(name_buf)))
                    {
                        filter.name = name_buf;
                    }
                    ImGui::PopItemWidth(); // input text width
                    ImGui::PopStyleColor(2); // input text and background colors

                    ImGui::Separator();

                    if (ImGui::Button(ICON_CI_PLUS))
                    {
                        // TODO: implement add component
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("Add Component");
                    }

                    ImGui::SameLine();
                    bool is_active{filter[EFilterFlag::IsActive]};
                    if (ImGui::Checkbox("Active", &is_active))
                    {
                        filter[EFilterFlag::IsActive] = is_active;
                    }

                    ImGui::SameLine();
                    bool is_highlight_only{filter[EFilterFlag::IsHighlightOnly]};
                    if (ImGui::Checkbox("Highlight Only", &is_highlight_only))
                    {
                        filter[EFilterFlag::IsHighlightOnly] = is_highlight_only;
                    }

                    ImGui::Separator();

                    int component_render_index{0};
                    for (auto& component : filters.components)
                    {
                        if (auto const component_it = std::find_if(
                                filter.component_ids.cbegin(),
                                filter.component_ids.cend(),
                                [&target_id = component.id](auto const& component_id) {
                                    return target_id == component_id;
                                });
                            component_it == filter.component_ids.cend())
                        {
                            continue;
                        }

                        LOG_TRACE(
                            "Tab {} | Filter {} | Component {}",
                            tab.name,
                            filter.name,
                            component.id.toString());

                        auto const component_child_id{std::to_string(++component_render_index)};
                        ImGui::BeginChild(
                            component_child_id.c_str(), ImVec2{0, 0}, ImGuiChildFlags_AutoResizeY);

                        ImGui::PushStyleColor(ImGuiCol_Button, {0.75f, 0.0f, 0.0f, 0.6f});
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.75f, 0.0f, 0.0f, 0.9f});
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.75f, 0.0f, 0.0f, 0.3f});
                        if (ImGui::Button(ICON_CI_TRASH))
                        {
                            // TODO: implement delete component
                        }
                        ImGui::PopStyleColor(3);
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("Delete Component");
                        }

                        // TODO: update when plugin implementation supports headers with IDs
                        const char* items[] = {"Option 1", "Option 2", "Option 3"};
                        static int selected_index = 0; // stores currently selected index
                        ImGui::SameLine();
                        ImGui::PushItemWidth(125);
                        if (ImGui::BeginCombo("##alternative", items[selected_index]))
                        {
                            for (int select_index = 0; select_index < IM_ARRAYSIZE(items);
                                 select_index++)
                            {
                                bool is_selected = (selected_index == select_index);
                                if (ImGui::Selectable(items[select_index], is_selected))
                                {
                                    selected_index = select_index;
                                }
                                if (is_selected)
                                {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::PopItemWidth();

                        using EFilterComponentFlag = Fluxion::API::Data::EFilterComponentFlag;

                        ImGui::SameLine();
                        bool const is_regex{component[EFilterComponentFlag::IsRegex]};
                        PushButtonGrayIfOff(is_regex);
                        if (ImGui::Button(ICON_CI_REGEX))
                        {
                            component[EFilterComponentFlag::IsRegex] = !is_regex;
                        }
                        PopButtonGrayIfOff(is_regex);
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip(is_regex ? "Toggle Regex Off" : "Toggle Regex On");
                        }

                        ImGui::SameLine();
                        bool const is_case_sensitive{component[EFilterComponentFlag::IsCaseSensitive]};
                        PushButtonGrayIfOff(is_case_sensitive);
                        if (ImGui::Button(ICON_CI_CASE_SENSITIVE))
                        {
                            component[EFilterComponentFlag::IsCaseSensitive] = !is_case_sensitive;
                        }
                        PopButtonGrayIfOff(is_case_sensitive);
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip(
                                is_case_sensitive ? "Toggle CaseSensitive Off"
                                                  : "Toggle CaseSensitive On");
                        }

                        ImGui::SameLine();
                        bool const is_equals{component[EFilterComponentFlag::IsEquals]};
                        PushButtonGrayIfOff(is_equals);
                        if (ImGui::Button(ICON_CI_CHEVRON_RIGHT))
                        {
                            component[EFilterComponentFlag::IsEquals] = !is_equals;
                        }
                        PopButtonGrayIfOff(is_equals);
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip(is_equals ? "Toggle Equals Off" : "Toggle Equals On");
                        }

                        ImGui::EndChild();
                    }

                    ImGui::EndChild();
                }
                ImGui::Separator();

                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }
}

} // namespace Fluxion::Application::Layers
