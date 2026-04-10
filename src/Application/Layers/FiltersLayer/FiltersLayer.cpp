/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.35
/// @brief Implementation of @see FiltersLayer.hpp.
///

#include <cstring>

#include "IconsCodicons.h"
#include "imgui.h"

#include "FiltersLayer.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"

#include "Fluxion/Application/Data/Formatters.hpp" // IWYU pragma: keep

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::FiltersLayer);
USE_LOG_SCOPE(Fluxion::Application::Layers::FiltersLayer);

namespace Fluxion::Application::Layers {

using namespace Fluxion::Application::Data;

namespace UIHelpers {

bool ColorsPicker(
    const char* id,
    Fluxion::API::Data::Common::Highlight& colors,
    ImVec4 const& display,
    std::string_view const preview)
{
    static bool s_editing_fg = true;

    if (ImGui::ColorButton(id, display))
    {
        ImGui::OpenPopup(id);
    }

    bool modified{false};
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
            modified = true;
        }

        ImGui::EndPopup();
    }
    return modified;
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

enum class ERedButtonType : std::uint8_t
{
    Button = 0,
    TabItemButton = 1
};

template <ERedButtonType TRedButtonType = ERedButtonType::Button>
void PushRedButton()
{
    if constexpr (TRedButtonType == ERedButtonType::Button)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, {0.75f, 0.0f, 0.0f, 0.6f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.75f, 0.0f, 0.0f, 0.9f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.75f, 0.0f, 0.0f, 0.3f});
    }
    else if constexpr (TRedButtonType == ERedButtonType::TabItemButton)
    {
        ImGui::PushStyleColor(ImGuiCol_Tab, {0.75f, 0.0f, 0.0f, 0.6f});
        ImGui::PushStyleColor(ImGuiCol_TabHovered, {0.75f, 0.0f, 0.0f, 0.9f});
        ImGui::PushStyleColor(ImGuiCol_TabActive, {0.75f, 0.0f, 0.0f, 0.3f});
    }
    else
    {
        static_assert(false, "Not supported ERedButtonType");
    }
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
    LOG_SCOPE("::FiltersLayer()");
}

void FiltersLayer::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
}

void FiltersLayer::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
    auto& app_state{m_application->GetApplicationState()};

    app_state.filters.id_to_metadata_updates.SyncFrontBufferSwap();
    for (auto& update : app_state.filters.id_to_metadata_updates.GetFront())
    {
        app_state.filters.id_to_metadata.emplace(update.first, std::move(update.second));
    }

    app_state.logs.searched_log.SyncFrontBufferCopy();
}

void FiltersLayer::OnRender()
{
    LOG_SCOPE("::OnRender()");

    auto& app_state{m_application->GetApplicationState()};

    app_state.filters.metadata.SyncFrontBufferCopy();

    app_state.filters.tabs.SyncFrontBufferCopy();
    for (auto& tab : app_state.filters.tabs.GetFront())
    {
        tab->filters.SyncFrontBufferCopy();
        for (auto& filter : tab->filters.GetFront())
        {
            filter->conditions.SyncFrontBufferCopy();
        }
    }

    ImGui::Begin(
        ICON_CI_WAND " Filters",
        &app_state.layers_active.filters,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    RenderTabs();

    ImGui::End();
}

void FiltersLayer::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
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

void FiltersLayer::MarkFiltersMetadataDirty()
{
    m_application->GetApplicationState().filters.metadata.UpdateBackBufferCopyLocking(
        [](Filters::FiltersGeneralMetadata& metadata) {
            metadata[Filters::EFiltersMetadataFlag::Applied] = false;
            metadata[Filters::EFiltersMetadataFlag::SavedToDisk] = false;
        });
}

void FiltersLayer::RenderToolbar()
{
    auto& app_state{m_application->GetApplicationState()};

    ImGui::BeginDisabled(
        app_state.filters.metadata.GetFront()[Filters::EFiltersMetadataFlag::SavedToDisk]);
    Graphite::Common::UI::TabItemIconButton(ICON_CI_SAVE, "Save Filters", [&]() {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::SaveFilters,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = std::nullopt,
                 .filter_id = std::nullopt,
                 .condition_id = std::nullopt,
             }});
    });
    ImGui::EndDisabled();

    Graphite::Common::UI::TabItemIconButton(ICON_CI_NEW_FOLDER, "Add Tab", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::AddTab,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = std::nullopt, .filter_id = std::nullopt, .condition_id = std::nullopt}});
        MarkFiltersMetadataDirty();
    });

    ImGui::BeginDisabled(app_state.filters.metadata.GetFront()[Filters::EFiltersMetadataFlag::Applied]);
    Graphite::Common::UI::TabItemIconButton(ICON_CI_WAND, "Apply Filters", [&]() {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::ApplyFilters,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = std::nullopt,
                 .filter_id = std::nullopt,
                 .condition_id = std::nullopt,
             }});
    });
    ImGui::EndDisabled();

    UIHelpers::Styles::PushRedButton<UIHelpers::Styles::ERedButtonType::TabItemButton>();
    Graphite::Common::UI::TabItemIconButton(ICON_CI_MUTE, "Disable Filters", [&]() {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::DisableFilters,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = std::nullopt,
                 .filter_id = std::nullopt,
                 .condition_id = std::nullopt,
             }});
    });
    UIHelpers::Styles::PopRedButton();
}

void FiltersLayer::RenderTabs()
{
    LOG_SCOPE("::RenderTabs()");
    auto& app_state{m_application->GetApplicationState()};
    // [!] Use this cautiously, because logging all tabs might drop performance in debug mode
    // LOG_TRACE("::RenderTabs(): Tabs: {}", app_state.filters.tabs.GetFront());

    if (ImGui::BeginTabBar("Tabs"))
    {
        RenderToolbar();

        for (auto& tab : app_state.filters.tabs.GetFront())
        {
            if (ImGui::BeginTabItem(tab->imgui_id.c_str()))
            {
                LOG_TRACE("::RenderTabs(): Rendering filter tab {}", tab->imgui_id.c_str());
                ImGui::Dummy(ImVec2(0.0f, 4.0f));

                RenderTab(tab);

                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void FiltersLayer::RenderTab(std::shared_ptr<Filters::Tab> tab_ptr)
{
    GRAPHITE_ASSERT(tab_ptr != nullptr, "Received tab::nullptr for rendering...");
    GRAPHITE_ASSERT(
        tab_ptr->id != Graphite::Common::Utility::UniqueID::Default(),
        "Received tab with default ID for rendering...");

    auto& tab{*tab_ptr};
    LOG_SCOPE("::RenderTab(): ID: \"{}\" | \"{}\"", tab.id, tab.name);

    Graphite::Common::UI::IconButton(ICON_CI_PLUS, "Add Filter", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::AddFilter,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = tab_ptr->id, .filter_id = std::nullopt, .condition_id = std::nullopt}});
    });

    ImGui::SameLine();
    Graphite::Common::UI::IconButton(ICON_CI_COPY, "Duplicate Tab", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::DuplicateTab,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = tab_ptr->id, .filter_id = std::nullopt, .condition_id = std::nullopt}});
    });

    ImGui::SameLine();
    UIHelpers::Styles::PushRedButton();
    Graphite::Common::UI::IconButton(ICON_CI_TRASH, "Delete Tab", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::RemoveTab,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = tab_ptr->id, .filter_id = std::nullopt, .condition_id = std::nullopt}});
    });
    UIHelpers::Styles::PopRedButton();

    ImGui::SameLine();
    bool is_active{tab[Filters::ETabFlag::IsActive]};
    if (ImGui::Checkbox("Active", &is_active))
    {
        tab[Filters::ETabFlag::IsActive] = is_active;
        MarkFiltersMetadataDirty();
    }

    ImGui::SameLine();
    if (Graphite::Common::UI::InputText("##tab_name", tab.name))
    {
        tab.UpdateImGuiID();
        MarkFiltersMetadataDirty();
    }

    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::BeginChild("##tab_filters_content", ImVec2(0, 0));
    for (auto& filter : tab.filters.GetFront())
    {
        RenderFilter(tab_ptr->id, *filter);

        ImGui::Separator();
    }
    ImGui::EndChild();
}

void FiltersLayer::RenderFilter(
    Graphite::Common::Utility::UniqueID const& owning_tab_id,
    Filters::Filter& filter)
{
    GRAPHITE_ASSERT(
        filter.id != Graphite::Common::Utility::UniqueID::Default(),
        "Received filter with default ID for rendering...");
    GRAPHITE_ASSERT(
        owning_tab_id != Graphite::Common::Utility::UniqueID::Default(),
        "Received owning_tab as default ID for rendering...");

    LOG_SCOPE("::RenderFilter(): ID: \"{}\" | \"{}\"", filter.id, filter.name);

    char s_filter_id[Graphite::Common::Utility::UniqueID::GetMinDumpSize()];
    filter.id.Dump(s_filter_id);
    ImGui::BeginChild(s_filter_id, ImVec2{0, 0}, ImGuiChildFlags_AutoResizeY);

    using EFilterFlag = Filters::EFilterFlag;

    UIHelpers::Styles::PushButtonGripper();
    Graphite::Common::UI::IconButton(ICON_CI_GRIPPER, "Move Filter", [&] {});
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            "FILTER_PAYLOAD", &filter.id, sizeof(Graphite::Common::Utility::UniqueID));
        ImGui::PushStyleColor(ImGuiCol_Text, filter.colors.foreground);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, filter.colors.background);
        ImGui::Text("%s", filter.name.c_str());
        ImGui::PopStyleColor(2);
        ImGui::EndDragDropSource();
    }
    UIHelpers::Styles::PopButtonGripper();

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILTER_PAYLOAD"))
        {
            auto& dragged_filter_id =
                *static_cast<const Graphite::Common::Utility::UniqueID*>(payload->Data);
            if (dragged_filter_id != filter.id)
            {
                Dispatch(
                    {.type = Actions::FiltersLayer::EFilterActionType::MoveFilter,
                     .data = Actions::FiltersLayer::Payloads::MoveFilter{
                         .tab_id = owning_tab_id,
                         .filter_id = dragged_filter_id,
                         .target_filter_id = filter.id}});
                MarkFiltersMetadataDirty();
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SameLine();
    bool const is_collapsed{filter[EFilterFlag::IsCollapsed]};
    Graphite::Common::UI::IconButton(
        is_collapsed ? ICON_CI_EYE_CLOSED : ICON_CI_EYE,
        is_collapsed ? "Show Filter" : "Hide Filter",
        [&] { filter[EFilterFlag::IsCollapsed] = !is_collapsed; });

    ImGui::SameLine();
    Graphite::Common::UI::IconButton(ICON_CI_COPY, "Duplicate Filter", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::DuplicateFilter,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = owning_tab_id, .filter_id = filter.id, .condition_id = std::nullopt}});
    });

    ImGui::SameLine();
    UIHelpers::Styles::PushRedButton();
    Graphite::Common::UI::IconButton(ICON_CI_TRASH, "Delete Filter", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::RemoveFilter,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = owning_tab_id, .filter_id = filter.id, .condition_id = std::nullopt}});
    });
    UIHelpers::Styles::PopRedButton();

    ImGui::SameLine();
    if (UIHelpers::ColorsPicker(
            "##FG", filter.colors, filter.colors.foreground, "Lorem ipsum dolor sit amet"))
    {
        m_application->GetApplicationState().filters.id_to_metadata[filter.id].colors = filter.colors;
    }
    ImGui::SameLine();
    if (UIHelpers::ColorsPicker(
            "##BG", filter.colors, filter.colors.background, "Lorem ipsum dolor sit amet"))
    {
        m_application->GetApplicationState().filters.id_to_metadata[filter.id].colors = filter.colors;
    }

    ImGui::SameLine();
    Graphite::Common::UI::VerticalSeparator();

    ImGui::SameLine();
    int priority_tmp = static_cast<int>(filter.priority);
    ImGui::PushItemWidth(85);
    if (ImGui::InputInt("##prio", &priority_tmp, 1, 1))
    {
        filter.priority = static_cast<std::uint8_t>(std::clamp(priority_tmp, 0, 255));
        MarkFiltersMetadataDirty();
    }
    ImGui::PopItemWidth();
    Graphite::Common::UI::ItemHoverTooltip("Filter Priority");

    ImGui::SameLine();
    Graphite::Common::UI::IconButton(ICON_CI_CHEVRON_LEFT, "Prev", [&]() {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::PrevLog,
             .data = Actions::FiltersLayer::Payloads::SearchLog{.filter_id = filter.id}});
    });
    ImGui::SameLine();
    Graphite::Common::UI::IconButton(ICON_CI_CHEVRON_RIGHT, "Next", [&]() {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::NextLog,
             .data = Actions::FiltersLayer::Payloads::SearchLog{.filter_id = filter.id}});
    });

    ImGui::SameLine();
    Graphite::Common::UI::VerticalSeparator();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, filter.colors.foreground);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, filter.colors.background);
    if (Graphite::Common::UI::InputText("##filter_name", filter.name))
    {
        MarkFiltersMetadataDirty();
    }
    ImGui::PopStyleColor(2); // input text and background colors

    ImGui::Separator();

    ImGui::Indent(32.0f);
    Graphite::Common::UI::IconButton(ICON_CI_PLUS, "Add Condition", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::AddCondition,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = owning_tab_id, .filter_id = filter.id, .condition_id = std::nullopt}});
    });

    ImGui::SameLine();
    bool is_active{filter[EFilterFlag::IsActive]};
    if (ImGui::Checkbox("Active", &is_active))
    {
        filter[EFilterFlag::IsActive] = is_active;
        MarkFiltersMetadataDirty();
    }

    ImGui::SameLine();
    bool is_highlight_only{filter[EFilterFlag::IsHighlightOnly]};
    if (ImGui::Checkbox("Highlight Only", &is_highlight_only))
    {
        filter[EFilterFlag::IsHighlightOnly] = is_highlight_only;
        MarkFiltersMetadataDirty();
    }

    ImGui::Separator();

    for (auto& condition_ptr : filter.conditions.GetFront())
    {
        RenderCondition(owning_tab_id, filter.id, *condition_ptr);
    }

    ImGui::EndChild();
}

void FiltersLayer::RenderCondition(
    Graphite::Common::Utility::UniqueID const& owning_tab_id,
    Graphite::Common::Utility::UniqueID const& owning_filter_id,
    Filters::Condition& condition)
{
    GRAPHITE_ASSERT(
        condition.id != Graphite::Common::Utility::UniqueID::Default(),
        "Received condition with default ID for rendering...");
    GRAPHITE_ASSERT(
        owning_tab_id != Graphite::Common::Utility::UniqueID::Default(),
        "Received owning_tab_id as default ID for rendering...");
    GRAPHITE_ASSERT(
        owning_filter_id != Graphite::Common::Utility::UniqueID::Default(),
        "Received owning_filter_id as default ID for rendering...");

    LOG_SCOPE("::RenderCondition(): ID: \"{}\"", condition.id);

    char s_condition_id[Graphite::Common::Utility::UniqueID::GetMinDumpSize()];
    condition.id.Dump(s_condition_id);
    ImGui::BeginChild(s_condition_id, ImVec2{0, 0}, ImGuiChildFlags_AutoResizeY);

    UIHelpers::Styles::PushButtonGripper();
    Graphite::Common::UI::IconButton(ICON_CI_GRIPPER, "Move Condition", [&] {});
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            "CONDITION_PAYLOAD", &condition.id, sizeof(Graphite::Common::Utility::UniqueID));

        auto const& header = m_application->GetApplicationState().logs.table_header;
        std::string column_name = "Unknown";
        for (auto const& col : header)
        {
            if (col.id == condition.over_column_id)
            {
                column_name = col.display_name;
                break;
            }
        }

        using EConditionFlag = Filters::EConditionFlag;
        std::string flags;
        if (condition[EConditionFlag::IsRegex])
            flags += "R";
        if (condition[EConditionFlag::IsEquals])
            flags += "E";
        if (condition[EConditionFlag::IsCaseSensitive])
            flags += "C";

        ImGui::TextUnformatted(column_name.c_str());
        ImGui::Text("%s", condition.data.c_str());
        if (!flags.empty())
        {
            ImGui::TextDisabled("[%s]", flags.c_str());
        }
        ImGui::EndDragDropSource();
    }
    UIHelpers::Styles::PopButtonGripper();

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONDITION_PAYLOAD"))
        {
            auto& dragged_condition_id =
                *static_cast<const Graphite::Common::Utility::UniqueID*>(payload->Data);
            if (dragged_condition_id != condition.id)
            {
                Dispatch(
                    {.type = Actions::FiltersLayer::EFilterActionType::MoveCondition,
                     .data = Actions::FiltersLayer::Payloads::MoveCondition{
                         .tab_id = owning_tab_id,
                         .filter_id = owning_filter_id,
                         .condition_id = dragged_condition_id,
                         .target_condition_id = condition.id}});
                MarkFiltersMetadataDirty();
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SameLine();
    UIHelpers::Styles::PushRedButton();
    Graphite::Common::UI::IconButton(ICON_CI_TRASH, "Delete Condition", [&] {
        Dispatch(
            {.type = Actions::FiltersLayer::EFilterActionType::RemoveCondition,
             Actions::FiltersLayer::Payloads::FiltersDataModify{
                 .tab_id = owning_tab_id, .filter_id = owning_filter_id, .condition_id = condition.id}});
    });
    UIHelpers::Styles::PopRedButton();

    auto const& header = m_application->GetApplicationState().logs.table_header;

    std::optional<std::size_t> selected_index{};
    for (std::size_t i = 0; i < header.size(); ++i)
    {
        if (header[i].id == condition.over_column_id)
        {
            selected_index = i;
            break;
        }
    }
    if (!static_cast<bool>(selected_index) && !header.empty())
    {
        selected_index = 0;
        condition.over_column_id = header.front().id;
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(125);
    const char* preview_value = header.empty() || !static_cast<bool>(selected_index)
                                    ? "None"
                                    : header[*selected_index].display_name.c_str();
    if (ImGui::BeginCombo("##alternative", preview_value))
    {
        for (std::size_t select_index = 0; select_index < header.size(); ++select_index)
        {
            bool is_selected = (*selected_index == select_index);
            if (ImGui::Selectable(header[select_index].display_name.c_str(), is_selected))
            {
                selected_index = select_index;
                condition.over_column_id = header[select_index].id;
                MarkFiltersMetadataDirty();
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    using EConditionFlag = Filters::EConditionFlag;

    ImGui::SameLine();
    bool const is_case_sensitive{condition[EConditionFlag::IsCaseSensitive]};
    UIHelpers::Styles::PushButtonGrayIfOff(is_case_sensitive);
    if (Graphite::Common::UI::IconButton(
            ICON_CI_CASE_SENSITIVE,
            is_case_sensitive ? "Toggle CaseSensitive Off" : "Toggle CaseSensitive On",
            [&] { condition[EConditionFlag::IsCaseSensitive] = !is_case_sensitive; }))
    {
        MarkFiltersMetadataDirty();
    };
    UIHelpers::Styles::PopButtonGrayIfOff(is_case_sensitive);

    ImGui::SameLine();
    bool const is_regex{condition[EConditionFlag::IsRegex]};
    UIHelpers::Styles::PushButtonGrayIfOff(is_regex);
    if (Graphite::Common::UI::IconButton(
            ICON_CI_REGEX, is_regex ? "Toggle Regex Off" : "Toggle Regex On", [&] {
                condition[EConditionFlag::IsRegex] = !is_regex;
            }))
    {
        MarkFiltersMetadataDirty();
    };
    UIHelpers::Styles::PopButtonGrayIfOff(is_regex);

    ImGui::SameLine();
    bool const is_equals{condition[EConditionFlag::IsEquals]};
    UIHelpers::Styles::PushButtonGrayIfOff(is_equals);
    if (Graphite::Common::UI::IconButton(
            ICON_CI_THREE_BARS, is_equals ? "Toggle Equals Off" : "Toggle Equals On", [&] {
                condition[EConditionFlag::IsEquals] = !is_equals;
            }))
    {
        MarkFiltersMetadataDirty();
    };
    UIHelpers::Styles::PopButtonGrayIfOff(is_equals);

    ImGui::SameLine();
    if (Graphite::Common::UI::InputText("##condition_data", condition.data))
    {
        MarkFiltersMetadataDirty();
    }

    ImGui::EndChild();
}

} // namespace Fluxion::Application::Layers
