/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RenderMenu.cpp
/// @author Alexandru Delegeanu
/// @version 5.4
/// @brief Implementation @see RegexTags.hpp
///

#include <string>

#include "IconsCodicons.h"

#include "Fluxion/Plugins/Logs/Text/RegexTags/V5/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::RenderMenu);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::RenderMenu);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5 {

namespace Utility {

std::string ConcatRegex(Data::RegexTags const& tags)
{
    std::string out{};

    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            out += "(" + tag->regex_data + ")";
        }
        else
        {
            out += tag->regex_data;
        }
    }

    return out;
}

inline ImVec4 GetCaptureGroupColor(std::size_t const group_index)
{
    float const hue =
        std::fmod(std::fmod(static_cast<float>(group_index) * 4.2069911f, 3.14159f), 1.0f);
    float r{0.0f}, g{0.0f}, b{0.0f};
    ImGui::ColorConvertHSVtoRGB(hue, 0.75f, 0.95f, r, g, b);
    return ImVec4(r, g, b, 1.0f);
}

void RenderRegexTagsPreview(Data::RegexTags const& tags, float const width = -1.0f)
{
    float const target_width = (width <= 0.0f) ? ImGui::GetContentRegionAvail().x : width;
    ImVec2 const frame_size(target_width, ImGui::GetFrameHeight());

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImGui::GetStyle().FramePadding);
    if (ImGui::BeginChild(
            "##RegexTagsPreview", frame_size, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoScrollbar))
    {
        std::size_t capture_group_idx{0};
        bool is_first_segment{true};

        ImVec4 const not_visible_tag_color{1.0f, 1.0f, 1.0f, 1.0f};

        for (auto const& tag : tags)
        {
            if (!tag || tag->regex_data.empty())
            {
                continue;
            }

            if (!is_first_segment)
            {
                ImGui::SameLine(0.0f, 0.0f);
            }
            is_first_segment = false;

            if (tag->visible)
            {
                ImVec4 const group_color = GetCaptureGroupColor(capture_group_idx++);

                // Darken the group color for parentheses (25% darker)
                ImVec4 const paren_color{
                    group_color.x * 0.75f, group_color.y * 0.75f, group_color.z * 0.75f, group_color.w};

                ImGui::TextColored(paren_color, "(");
                ImGui::SameLine(0.0f, 0.0f);
                ImGui::TextColored(group_color, "%s", tag->regex_data.c_str());
                ImGui::SameLine(0.0f, 0.0f);
                ImGui::TextColored(paren_color, ")");
            }
            else
            {
                ImGui::TextColored(not_visible_tag_color, "%s", tag->regex_data.c_str());
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

} // namespace Utility

void RegexTags::RenderMenu()
{
    LOG_SCOPE("::RenderMenu()");

    bool tags_dirty{false};

    ImGui::TextUnformatted(ICON_CI_TOOLS " Regex Configurator");

    m_regex_tags.SyncFrontBufferCopy();

    Graphite::Common::UI::IconButton(ICON_CI_REPO_PULL, "Import", []() {});
    ImGui::SameLine();
    Graphite::Common::UI::IconButton(ICON_CI_REPO_PUSH, "Export", []() {});
    ImGui::SameLine();
    Utility::RenderRegexTagsPreview(m_regex_tags.GetFront());

    ImGui::BeginDisabled(m_logs_operation_progress != 0);
    if (ImGui::BeginTable(
            "##regex-configurator", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
    {
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Tag Name", ImGuiTableColumnFlags_WidthStretch, 0.35f);
        ImGui::TableSetupColumn("Tag Regex", ImGuiTableColumnFlags_WidthStretch, 0.65f);

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        Graphite::Common::UI::IconButton(ICON_CI_ADD, "Add", [&]() {
            m_regex_tags.UpdateBackBufferCopy([&tags_dirty](Data::RegexTags& back_tags) {
                LOG_INFO("::RenderMenu(): Add Tag.");
                tags_dirty = true;
                auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
                new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
                LOG_INFO("::RenderMenu(): New Tag ID {}.", new_tag->id);

                new_tag->display_name = "New Tag";
                new_tag->regex_data = ".*";
                new_tag->visible = true;
            });
        });
        ImGui::SameLine();
        Graphite::Common::UI::IconButton(
            ICON_CI_WAND, "Apply", []() { LOG_INFO("::RenderMenu(): Apply Tags."); });

        ImGui::TableNextColumn();
        static char s_tag_name[]{"Tag Name"};
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##tag-name", s_tag_name, sizeof(s_tag_name), ImGuiInputTextFlags_ReadOnly);

        ImGui::TableNextColumn();
        static char s_tag_regex[]{"Tag Regex"};
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##tag-regex", s_tag_regex, sizeof(s_tag_regex), ImGuiInputTextFlags_ReadOnly);

        auto const& front_tags{m_regex_tags.GetFront()};
        for (std::size_t idx = 0; idx < front_tags.size(); ++idx)
        {
            auto& tag{*front_tags[idx]};

            ImGui::PushID(tag.id.ToHash<int>());

            ImGui::TableNextRow();

            ImGui::TableNextColumn();

            Graphite::Common::UI::IconButton(ICON_CI_TRASH, "Delete", [&]() {
                tags_dirty = true;
                m_regex_tags.UpdateBackBufferCopy([idx](Data::RegexTags& back_tags) {
                    if (idx < back_tags.size())
                    {
                        LOG_INFO(
                            "::RenderMenu(): Delete Tag {} {}",
                            back_tags.back()->id,
                            back_tags.back()->display_name);
                        back_tags.erase(
                            back_tags.begin() + static_cast<Data::RegexTags::difference_type>(idx));
                    }
                    else
                    {
                        LOG_INFO("::RenderMenu(): No Tag to be deleted.");
                    }
                });
            });

            ImGui::SameLine();
            Graphite::Common::UI::IconButton(
                tag.visible ? ICON_CI_EYE : ICON_CI_EYE_CLOSED,
                tag.visible ? "Toggle Visible: OFF" : "Toggle Visible: ON",
                [&]() {
                    tags_dirty = true;
                    m_regex_tags.UpdateBackBufferCopy([idx](Data::RegexTags& back_tags) {
                        if (idx < back_tags.size())
                        {
                            LOG_INFO(
                                "::RenderMenu(): Toggle Tag Visible {} {} to {}",
                                back_tags[idx]->id,
                                back_tags[idx]->display_name,
                                !back_tags[idx]->visible);
                            back_tags[idx]->visible = !back_tags[idx]->visible;
                        }
                    });
                });

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            if (Graphite::Common::UI::InputText("##regex-name", tag.display_name))
            {
                tags_dirty = true;
            };

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            if (Graphite::Common::UI::InputText("##regex-data", tag.regex_data))
            {
                tags_dirty = true;
            };

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    if (tags_dirty)
    {
        SaveRegexTags(m_regex_tags.GetFront());
    }

    static auto s_default_config{Data::Settings{}};

    if (ImGui::CollapsingHeader("Import Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginTable("##ImportSettingsTable", 3, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGui::TableSetupColumn("Reset", ImGuiTableColumnFlags_WidthFixed, 110.0f);

            auto render_input = [this](
                                    const char* label,
                                    const char* id,
                                    std::int32_t& value,
                                    std::int32_t const max_value,
                                    std::int32_t const default_value,
                                    const char* obj_key,
                                    const char* config_key) {
                ImGui::TableNextRow();

                // Column 0: Label
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(label);

                // Column 1: Input control
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-1); // Fill available column width (110px)

                std::optional<std::int32_t> opt_value{std::nullopt};

                if (ImGui::InputInt(id, &value, 1))
                {
                    opt_value = std::clamp(value, 1, max_value);
                }

                ImGui::TableNextColumn();
                ImGui::PushID(id);
                if (ImGui::Button("Reset"))
                {
                    opt_value = default_value;
                    value = default_value;
                }
                ImGui::PopID();

                if (static_cast<bool>(opt_value))
                {
                    value = *opt_value;

                    auto config{GetConfig()};
                    auto json = config.GetJsonValue(obj_key).value_or(nlohmann::json::object());

                    json[config_key] = *opt_value;

                    config.SetJsonValue(obj_key, json);
                    config.Save();
                }
            };

            render_input(
                "Workers Count",
                "##workerscount",
                m_settings.import_params.workers_count,
                s_default_config.import_params.workers_count * 100,
                s_default_config.import_params.workers_count,
                "import",
                "workers_count");
            render_input(
                "Batch Capacity",
                "##batchcapacity",
                m_settings.import_params.batch_capacity,
                s_default_config.import_params.batch_capacity * 100,
                s_default_config.import_params.batch_capacity,
                "import",
                "batch_capacity");
            render_input(
                "Available batches per worker",
                "##availablebpw",
                m_settings.import_params.available_batches_per_worker,
                s_default_config.import_params.available_batches_per_worker * 100,
                s_default_config.import_params.available_batches_per_worker,
                "import",
                "available_batches_per_worker");
            render_input(
                "Rows per transaction",
                "##rowspertransaction",
                m_settings.import_params.rows_per_transaction,
                s_default_config.import_params.rows_per_transaction * 100,
                s_default_config.import_params.rows_per_transaction,
                "import",
                "rows_per_transaction");

            ImGui::EndTable();
        }
    }
    ImGui::EndDisabled();
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5
