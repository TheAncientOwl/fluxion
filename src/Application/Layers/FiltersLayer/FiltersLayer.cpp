/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.16
/// @brief Implementation of @see FiltersLayer.hpp.
///

#include <cstring>

#include "IconsCodicons.h"
#include "imgui.h"

#include "FiltersLayer.hpp"

#include "Fluxion/API/DataIO.hpp"

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
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TSoftMenuCloseableLayer{std::move(application), z_index}
{
    LOG_SCOPE("");
}

void FiltersLayer::OnAdd()
{
    LOG_SCOPE("");
}

void FiltersLayer::OnIterate()
{
    LOG_SCOPE("");
}

void FiltersLayer::OnRender()
{
    LOG_SCOPE("");

    auto& app_state{m_application->GetApplicationState()};

    app_state.filters.tabs.SyncFront();
    for (auto& tab : app_state.filters.tabs.front)
    {
        tab->filters.SyncFront();
        for (auto& filter : tab->filters.front)
        {
            filter->components.SyncFront();
        }
    }

    ImGui::Begin(ICON_CI_WAND " Filters", &app_state.layers_active.filters);

    RenderFiltersTabs();

    ImGui::End();
}

void FiltersLayer::OnRemove()
{
    LOG_SCOPE("");
}

inline bool FiltersLayer::IsActive() const noexcept
{
    return m_application->GetApplicationState().layers_active.filters;
}

inline void FiltersLayer::SetIsActive(bool const open)
{
    m_application->GetApplicationState().layers_active.filters = open;
}

inline std::string_view FiltersLayer::GetDisplayName() const noexcept
{
    return "Filters";
}

void FiltersLayer::Dispatch(Actions::FiltersLayer::FilterActionPayload&& payload)
{
    m_application->PushAction(EFluxionAction::FilterAction, std::move(payload));
}

namespace UIHelpers {

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

enum class EInputTextWidth : std::uint8_t
{
    Auto,
    Fill
};

template <std::size_t TBufferSize, EInputTextWidth TInputTextWidth = EInputTextWidth::Fill>
void InputText(const char* label, std::string& str, bool& dirty)
{
    char buffer[TBufferSize];
    std::strncpy(buffer, str.c_str(), TBufferSize);
    buffer[TBufferSize - 1] = '\0';

    if constexpr (TInputTextWidth == EInputTextWidth::Fill)
    {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    }

    if (ImGui::InputText(label, buffer, TBufferSize))
    {
        str = buffer;
        dirty = true;
    }

    if constexpr (TInputTextWidth == EInputTextWidth::Fill)
    {
        ImGui::PopItemWidth();
    }
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
    std::string_view const preview,
    bool& dirty)
{
    static bool s_editing_fg = true;

    if (ImGui::ColorButton(id, display))
    {
        ImGui::OpenPopup(id);
    }

    if (ImGui::BeginPopup(id))
    {
        // --- Checkbox to switch FG/BG ---
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

        if (ImGui::ColorPicker4(
                "##color", reinterpret_cast<float*>(&target), ImGuiColorEditFlags_NoSidePreview))
        {
            dirty = true;
        }

        ImGui::EndPopup();
    }
}

namespace Styles {

void PushButtonGrayIfOff(bool const state)
{
    if (!state)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, {0.5f, 0.5f, 0.5f, 0.6f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.6f, 0.6f, 0.6f, 0.9f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.4f, 0.4f, 0.4f, 0.3f});
    }
};

void PopButtonGrayIfOff(bool const state)
{
    if (!state)
    {
        ImGui::PopStyleColor(3);
    }
};

void PushRedButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, {0.75f, 0.0f, 0.0f, 0.6f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.75f, 0.0f, 0.0f, 0.9f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.75f, 0.0f, 0.0f, 0.3f});
};

void PopRedButton()
{
    ImGui::PopStyleColor(3);
};

void PushButtonGripper()
{
    ImGui::PushStyleColor(ImGuiCol_Button, {0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.0f, 0.0f, 0.0f, 0.0f});
}

void PopButtonGripper()
{
    ImGui::PopStyleColor(3);
}

} // namespace Styles

} // namespace UIHelpers

void FiltersLayer::RenderFiltersTabs()
{
    LOG_SCOPE("");
    auto& app_state{m_application->GetApplicationState()};
    // [!] Use this cautiously, because logging all tabs might drop performance in debug mode
    LOG_DEBUG("FiltersTabs: {}", Fluxion::Utils::Format::format_vector(app_state.filters.tabs.front));

    if (ImGui::BeginTabBar("FiltersTabs"))
    {
        for (auto& tab : app_state.filters.tabs.front)
        {
            auto const tab_label = tab->name + "###" + tab->id;
            if (ImGui::BeginTabItem(tab_label.c_str()))
            {
                LOG_INFO("Rendering filter tab {}", tab_label);
                RenderFiltersTab(tab, app_state.filters.dirty);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void FiltersLayer::RenderFiltersTab(std::shared_ptr<Fluxion::API::Data::FiltersTab> tab_ptr, bool& dirty)
{
    GRAPHITE_ASSERT(tab_ptr != nullptr, "Received tab::nullptr for rendering...");
    GRAPHITE_ASSERT(
        tab_ptr->id != Graphite::Common::UniqueID::Default(),
        "Received tab with default ID for rendering...");

    auto& tab{*tab_ptr};
    LOG_SCOPE("ID: \"{}\" | \"{}\"", tab.id, tab.name);

    if (ImGui::Button(ICON_CI_PLUS))
    {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::AddFilter,
             .tab_id = tab_ptr->id,
             .filter_id = std::nullopt,
             .component_id = std::nullopt});
    }
    UIHelpers::ItemHoverTooltip("Add Filter");

    ImGui::SameLine();
    if (ImGui::Button(ICON_CI_NEW_FOLDER))
    {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::AddFiltersTab,
             .tab_id = std::nullopt,
             .filter_id = std::nullopt,
             .component_id = std::nullopt});
    }
    UIHelpers::ItemHoverTooltip("Add Tab");

    ImGui::SameLine();
    if (ImGui::Button(ICON_CI_COPY))
    {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::DuplicateFiltersTab,
             .tab_id = tab_ptr->id,
             .filter_id = std::nullopt,
             .component_id = std::nullopt});
    }
    UIHelpers::ItemHoverTooltip("Duplicate Tab");

    ImGui::SameLine();
    UIHelpers::Styles::PushRedButton();
    if (ImGui::Button(ICON_CI_TRASH))
    {
        dirty = true;
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::RemoveFiltersTab,
             .tab_id = tab_ptr->id,
             .filter_id = std::nullopt,
             .component_id = std::nullopt});
    }
    UIHelpers::Styles::PopRedButton();
    UIHelpers::ItemHoverTooltip("Delete Tab");

    ImGui::SameLine();
    bool is_active{tab[API::Data::EFiltersTabFlag::IsActive]};
    if (ImGui::Checkbox("Active", &is_active))
    {
        dirty = true;
        tab[API::Data::EFiltersTabFlag::IsActive] = is_active;
    }

    ImGui::SameLine();
    UIHelpers::InputText<128>("##tab_name", tab.name, dirty);

    for (auto& filter : tab.filters.front)
    {
        RenderFilter(tab_ptr->id, *filter, dirty);

        ImGui::Separator();
    }
}

void FiltersLayer::RenderFilter(
    Graphite::Common::UniqueID const& owning_tab_id,
    Fluxion::API::Data::Filter& filter,
    bool& dirty)
{
    // GRAPHITE_ASSERT(filter != nullptr, "Received filter::nullptr for rendering...");
    GRAPHITE_ASSERT(
        filter.id != Graphite::Common::UniqueID::Default(),
        "Received filter with default ID for rendering...");

    GRAPHITE_ASSERT(
        owning_tab_id != Graphite::Common::UniqueID::Default(),
        "Received owning_tab as default ID for rendering...");

    // auto& filter{*filter};
    LOG_SCOPE("ID: \"{}\" | \"{}\"", filter.id, filter.name);

    auto const filter_id{filter.id.toString()};
    ImGui::BeginChild(filter_id.c_str(), ImVec2{0, 0}, ImGuiChildFlags_AutoResizeY);
    ImGui::Separator();

    using EFilterFlag = Fluxion::API::Data::EFilterFlag;

    UIHelpers::Styles::PushButtonGripper();
    if (ImGui::Button(ICON_CI_GRIPPER))
    {
        // TODO: Implement move
    }
    UIHelpers::Styles::PopButtonGripper();
    UIHelpers::ItemHoverTooltip("Move Filter");

    ImGui::SameLine();
    bool const is_collapsed{filter[EFilterFlag::IsCollapsed]};
    if (ImGui::Button(is_collapsed ? ICON_CI_EYE_CLOSED : ICON_CI_EYE))
    {
        filter[EFilterFlag::IsCollapsed] = !is_collapsed;
    }
    UIHelpers::ItemHoverTooltip(is_collapsed ? "Show Filter" : "Hide Filter");

    ImGui::SameLine();
    if (ImGui::Button(ICON_CI_COPY))
    {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::DuplicateFilter,
             .tab_id = owning_tab_id,
             .filter_id = filter.id,
             .component_id = std::nullopt});
    }
    UIHelpers::ItemHoverTooltip("Duplicate Filter");

    ImGui::SameLine();
    UIHelpers::Styles::PushRedButton();
    if (ImGui::Button(ICON_CI_TRASH))
    {
        dirty = true;
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::RemoveFilter,
             .tab_id = owning_tab_id,
             .filter_id = filter.id,
             .component_id = std::nullopt});
    }
    UIHelpers::Styles::PopRedButton();
    UIHelpers::ItemHoverTooltip("Delete Filter");

    ImGui::SameLine();
    UIHelpers::ColorsPicker(
        "##FG", filter.colors, filter.colors.foreground, "Lorem ipsum dolor sit amet", dirty);
    ImGui::SameLine();
    UIHelpers::ColorsPicker(
        "##BG", filter.colors, filter.colors.background, "Lorem ipsum dolor sit amet", dirty);

    ImGui::SameLine();
    UIHelpers::VerticalSeparator();

    ImGui::SameLine();
    int priority_tmp = static_cast<int>(filter.priority);
    ImGui::PushItemWidth(85);
    if (ImGui::InputInt("##prio", &priority_tmp, 1, 1))
    {
        dirty = true;
        filter.priority = static_cast<std::uint8_t>(std::clamp(priority_tmp, 0, 255));
    }
    ImGui::PopItemWidth();
    UIHelpers::ItemHoverTooltip("Filter Priority");

    ImGui::SameLine();
    UIHelpers::VerticalSeparator();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, filter.colors.foreground);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, filter.colors.background);
    UIHelpers::InputText<128>("##filter_name", filter.name, dirty);
    ImGui::PopStyleColor(2); // input text and background colors

    ImGui::Separator();

    // ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 32.0f);
    ImGui::Indent(32.0f);
    if (ImGui::Button(ICON_CI_PLUS))
    {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::AddFilterComponent,
             .tab_id = owning_tab_id,
             .filter_id = filter.id,
             .component_id = std::nullopt});
    }
    UIHelpers::ItemHoverTooltip("Add Component");

    ImGui::SameLine();
    bool is_active{filter[EFilterFlag::IsActive]};
    if (ImGui::Checkbox("Active", &is_active))
    {
        dirty = true;
        filter[EFilterFlag::IsActive] = is_active;
    }

    ImGui::SameLine();
    bool is_highlight_only{filter[EFilterFlag::IsHighlightOnly]};
    if (ImGui::Checkbox("Highlight Only", &is_highlight_only))
    {
        dirty = true;
        filter[EFilterFlag::IsHighlightOnly] = is_highlight_only;
    }

    ImGui::Separator();

    ImGui::Indent(32.0f);
    for (auto& component_ptr : filter.components.front)
    {
        RenderFilterComponent(owning_tab_id, filter.id, *component_ptr, dirty);
    }

    ImGui::EndChild();
}

void FiltersLayer::RenderFilterComponent(
    Graphite::Common::UniqueID const& owning_tab_id,
    Graphite::Common::UniqueID const& owning_filter_id,
    Fluxion::API::Data::FilterComponent& component,
    bool& dirty)
{
    // GRAPHITE_ASSERT(component_ptr != nullptr, "Received component::nullptr for rendering...");
    GRAPHITE_ASSERT(
        component.id != Graphite::Common::UniqueID::Default(),
        "Received component with default ID for rendering...");

    GRAPHITE_ASSERT(
        owning_tab_id != Graphite::Common::UniqueID::Default(),
        "Received owning_tab_id as default ID for rendering...");
    GRAPHITE_ASSERT(
        owning_filter_id != Graphite::Common::UniqueID::Default(),
        "Received owning_filter_id as default ID for rendering...");

    // auto& component{*component_ptr};
    LOG_SCOPE("ID: \"{}\"", component.id);

    auto const component_id{component.id.toString()};
    ImGui::BeginChild(component_id.c_str(), ImVec2{0, 0}, ImGuiChildFlags_AutoResizeY);

    UIHelpers::Styles::PushButtonGripper();
    if (ImGui::Button(ICON_CI_GRIPPER))
    {
        // TODO: Implement move
    }
    UIHelpers::Styles::PopButtonGripper();
    UIHelpers::ItemHoverTooltip("Move Component");

    ImGui::SameLine();
    UIHelpers::Styles::PushRedButton();
    if (ImGui::Button(ICON_CI_TRASH))
    {
        dirty = true;
        // filters_tabs.RemoveComponent(component.id);
        // TODO:
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::RemoveFilterComponent,
             .tab_id = owning_tab_id,
             .filter_id = owning_filter_id,
             .component_id = component.id});
    }
    UIHelpers::Styles::PopRedButton();
    UIHelpers::ItemHoverTooltip("Delete Component");

    // TODO: update when plugin implementation supports headers with IDs
    constexpr std::size_t c_dummy_options_size{3};
    const char* items[c_dummy_options_size] = {"Option 1", "Option 2", "Option 3"};
    static std::size_t selected_index = 0; // stores currently selected index
    ImGui::SameLine();
    ImGui::PushItemWidth(125);
    if (ImGui::BeginCombo("##alternative", items[selected_index]))
    {
        for (std::size_t select_index = 0; select_index < c_dummy_options_size; ++select_index)
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
    UIHelpers::Styles::PushButtonGrayIfOff(is_regex);
    if (ImGui::Button(ICON_CI_REGEX))
    {
        dirty = true;
        component[EFilterComponentFlag::IsRegex] = !is_regex;
    }
    UIHelpers::Styles::PopButtonGrayIfOff(is_regex);
    UIHelpers::ItemHoverTooltip(is_regex ? "Toggle Regex Off" : "Toggle Regex On");

    ImGui::SameLine();
    bool const is_case_sensitive{component[EFilterComponentFlag::IsCaseSensitive]};
    UIHelpers::Styles::PushButtonGrayIfOff(is_case_sensitive);
    if (ImGui::Button(ICON_CI_CASE_SENSITIVE))
    {
        dirty = true;
        component[EFilterComponentFlag::IsCaseSensitive] = !is_case_sensitive;
    }
    UIHelpers::Styles::PopButtonGrayIfOff(is_case_sensitive);
    UIHelpers::ItemHoverTooltip(
        is_case_sensitive ? "Toggle CaseSensitive Off" : "Toggle CaseSensitive On");

    ImGui::SameLine();
    bool const is_equals{component[EFilterComponentFlag::IsEquals]};
    UIHelpers::Styles::PushButtonGrayIfOff(is_equals);
    if (ImGui::Button(ICON_CI_CHEVRON_RIGHT))
    {
        dirty = true;
        component[EFilterComponentFlag::IsEquals] = !is_equals;
    }
    UIHelpers::Styles::PopButtonGrayIfOff(is_equals);
    UIHelpers::ItemHoverTooltip(is_equals ? "Toggle Equals Off" : "Toggle Equals On");

    ImGui::SameLine();
    UIHelpers::InputText<256>("##component_data", component.data, dirty);

    ImGui::EndChild();
}

} // namespace Fluxion::Application::Layers
