/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTextV1LogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "IconsCodicons.h"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

std::string_view RegexTextV1LogsPlugin::GetDisplayName() const
{
    return "RegexTextV1";
}

void RegexTextV1LogsPlugin::OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& /*data*/)
{
    LOG_SCOPE("::OnEnable()");
    m_regex_tags.UpdateBackBufferCopy([](RegexTags& back_tags) {
        auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
        new_tag->display_name = "New Tag";
        new_tag->regex_data = ".*";
        new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
        new_tag->visible = true;
    });
}

void RegexTextV1LogsPlugin::OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/)
{
    LOG_SCOPE("::OnDisable()");
}

void RegexTextV1LogsPlugin::RenderMenu()
{
    LOG_SCOPE("::RenderMenu()");

    ImGui::TextUnformatted(ICON_CI_TOOLS " Regex Configurator");

    m_regex_tags.SyncFrontBufferCopy();

    Graphite::Common::UI::IconButton(ICON_CI_REPO_PULL, "Import", []() {});
    ImGui::SameLine();
    Graphite::Common::UI::IconButton(ICON_CI_REPO_PUSH, "Export", []() {});
    static char s_full_regex[]{"dawd"};
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
                auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
                new_tag->display_name = "New Tag";
                new_tag->regex_data = ".*";
                new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag->visible = true;
            });
        });
        ImGui::SameLine();
        Graphite::Common::UI::IconButton(ICON_CI_WAND, "Apply", []() {});

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
                        back_tags.erase(
                            back_tags.begin() + static_cast<RegexTags::difference_type>(idx));
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

void RegexTextV1LogsPlugin::ImportLogs(std::filesystem::path const& /*path*/)
{
    LOG_SCOPE("::ImportLogs()");
}

std::optional<std::size_t> RegexTextV1LogsPlugin::GetNextLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetNextLog()");
    return std::nullopt;
}

std::optional<std::size_t> RegexTextV1LogsPlugin::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetPrevLog()");
    return std::nullopt;
}

void RegexTextV1LogsPlugin::ApplyFilters(
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> /*filters*/,
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> /*highlight_only*/)
{
    LOG_SCOPE("::ApplyFilters()");
}

void RegexTextV1LogsPlugin::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
}

std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> RegexTextV1LogsPlugin::GetTableHeader() const
{
    return {};
}

std::size_t RegexTextV1LogsPlugin::GetTotalLogs() const
{
    return 0;
}

void RegexTextV1LogsPlugin::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& /*ranges*/,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter /*out_logs*/) const
{
    LOG_SCOPE("::GetLogs()");
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin()
{
    return new Fluxion::Plugins::Logs::RegexTextV1::RegexTextV1LogsPlugin();
}
