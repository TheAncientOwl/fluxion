/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RenderMenu.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <string>

#include "IconsCodicons.h"

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

void RegexTextV1LogsPlugin::RenderMenu()
{
    LOG_SCOPE("::RenderMenu()");

    ImGui::TextUnformatted(ICON_CI_TOOLS " Regex Configurator");

    m_regex_tags.SyncFrontBufferCopy();

    Graphite::Common::UI::IconButton(ICON_CI_REPO_PULL, "Import", []() {});
    ImGui::SameLine();
    Graphite::Common::UI::IconButton(ICON_CI_REPO_PUSH, "Export", []() {});
    static char s_full_regex[]{"TODO: Compute/Concat the full regex"};
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##full-regex", s_full_regex, sizeof(s_full_regex), ImGuiInputTextFlags_ReadOnly);

    if (ImGui::BeginTable(
            "##regex-configurator", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
    {
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Tag Name", ImGuiTableColumnFlags_WidthStretch, 0.35f);
        ImGui::TableSetupColumn("Tag Regex", ImGuiTableColumnFlags_WidthStretch, 0.65f);

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        Graphite::Common::UI::IconButton(ICON_CI_ADD, "Add", [&]() {
            m_regex_tags.UpdateBackBufferCopy([](RegexTags& back_tags) {
                LOG_INFO("::RenderMenu(): Add Tag.");
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
                m_regex_tags.UpdateBackBufferCopy([idx](RegexTags& back_tags) {
                    if (idx < back_tags.size())
                    {
                        LOG_INFO(
                            "::RenderMenu(): Delete Tag {} {}",
                            back_tags.back()->id,
                            back_tags.back()->display_name);
                        back_tags.erase(
                            back_tags.begin() + static_cast<RegexTags::difference_type>(idx));
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
                    m_regex_tags.UpdateBackBufferCopy([idx](RegexTags& back_tags) {
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
            Graphite::Common::UI::InputText("##regex-name", tag.display_name);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-1);
            Graphite::Common::UI::InputText("##regex-data", tag.regex_data);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1
