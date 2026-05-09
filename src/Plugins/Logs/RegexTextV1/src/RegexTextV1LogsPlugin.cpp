/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTextV1LogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <fstream>
#include <regex>
#include <string>

#include <exception>
#include "miocsv.h"
#include "stdcsv.h"

#include "IconsCodicons.h"

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

std::string_view RegexTextV1LogsPlugin::GetDisplayName() const
{
    return "RegexTextV1";
}

void RegexTextV1LogsPlugin::OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data)
{
    m_home_path = data.plugin_home_path;

    LOG_SCOPE("::OnEnable()");
    LOG_TRACE("::OnEnable()");
    m_regex_tags.UpdateBackBufferCopy([](RegexTags& back_tags) {
        //{ auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
        // new_tag->display_name = "New Tag";
        // new_tag->regex_data = ".*";
        // new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
        // new_tag->visible = true;}

        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Timestamp";
            new_tag->regex_data = R"(\d+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Channel";
            new_tag->regex_data = R"(Channel[1-4])";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Level";
            new_tag->regex_data = R"(trace|info|error|debug|warn)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Payload";
            new_tag->regex_data = R"(.*)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
    });
}

void RegexTextV1LogsPlugin::OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/)
{
    LOG_SCOPE("::OnDisable()");
    LOG_TRACE("::OnDisable()");
}

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

void RegexTextV1LogsPlugin::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("::ImportLogs()");
    LOG_INFO("Importing {}", path.c_str());

    m_regex_tags.SyncFrontBufferCopy();
    auto const& tags{m_regex_tags.GetFront()};

    std::string full_pattern{};
    m_imported_logs_header.clear();
    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            // TODO: generated IDs
            // TODO: save to config
            m_imported_logs_header.push_back(
                {Graphite::Common::Utility::UniqueID::Default(), tag->display_name});
            full_pattern += "(" + tag->regex_data + ")";
        }
        else
        {
            full_pattern += tag->regex_data;
        }
    }
    LOG_INFO("::ImportLogs(): Full regex pattern: {}", full_pattern);

    std::regex line_regex{};
    try
    {
        line_regex = std::regex(full_pattern);
    }
    catch (std::exception const& e)
    {
        LOG_WARN("Invalid regex: {}", e.what());
        return;
    }

    m_last_imported_logs_path = path;
    std::ifstream raw_logs_file{path};
    if (!raw_logs_file.is_open())
    {
        LOG_WARN("::ImportLogs(): Could not open file {}", path.c_str());
        return;
    }

    auto const output_path{MakeConvertedLogsPath(path)};
    auto writer = miocsv::Writer{output_path};
    LOG_INFO("Output CSV file {}", output_path.string());

    std::string line{};
    line.reserve(1024);
    std::size_t total_logs{0};

    while (std::getline(raw_logs_file, line))
    {
        std::smatch matches;
        if (std::regex_match(line, matches, line_regex))
        {
            ++total_logs;
            if (total_logs % 1000 == 0)
            {
                LOG_INFO("Read another 1000 chunk, total: {} / 2000000", total_logs);
            }

            // matches[0] is full match, start from 1 like Rust version
            miocsv::Row row{};
            for (std::size_t i = 1; i < matches.size(); ++i)
            {
                if (matches[i].matched)
                {
                    row.append(matches[i].str());
                    // LOG_TRACE("::ImportLogs(): Col {}: {}", i, matches[i].str());
                    // TODO: store or process column value
                    // e.g. LOG_TRACE("Col {}: {}", i, matches[i].str());
                }
            }
            writer.write_row(row);
        }
        else
        {
            LOG_WARN("::ImportLogs(): Regex did not match entry '{}'", line);
        }
    }

    LOG_INFO("::ImportLogs(): Total matched logs: {}", total_logs);
    auto settings{GetConfig()};
    settings.set("total_logs", total_logs);
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
    return m_imported_logs_header;
}

std::size_t RegexTextV1LogsPlugin::GetTotalLogs() const
{
    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    return static_cast<bool>(total_logs_opt) ? *total_logs_opt : 0;
}

void RegexTextV1LogsPlugin::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) const
{
    LOG_SCOPE("::GetLogs()");

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogs(): No logs were imported before");
        return;
    }

    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    if (!static_cast<bool>(total_logs_opt))
    {
        LOG_WARN("::GetLogs(): total_logs is not set in config");
    }

    auto const last_line_index{[&ranges]() {
        std::size_t last_idx{std::numeric_limits<std::size_t>::min()};
        for (auto& range : ranges)
        {
            last_idx = std::max(last_idx, range.end);
        }
        return last_idx;
    }()};

    auto reader = miocsv::MIOReader{MakeConvertedLogsPath(*m_last_imported_logs_path)};
    for (auto row : reader)
    {
        auto const row_num{reader.get_row_num() - 1};

        if (row_num > last_line_index || row_num > *total_logs_opt)
        {
            break;
        }

        if (!std::any_of(ranges.begin(), ranges.end(), [row_num](auto const& range) {
                return range.begin <= row_num && row_num < range.end;
            }))
        {
            continue;
        }

        auto& target_row = out_logs[row_num];
        if (target_row.data.size() < row.size())
        {
            target_row.data.resize(row.size());
        }

        for (std::size_t col_idx = 0; col_idx < row.size(); ++col_idx)
        {
            // TODO: check with move(row)
            target_row.data[col_idx] = row[col_idx];
        }

        // TODO: update with filters when implemented
        target_row.metadata = {};
    }
}

std::filesystem::path RegexTextV1LogsPlugin::MakeConvertedLogsPath(
    std::filesystem::path const& raw_logs_path) const
{
    auto const output_path{m_home_path / (raw_logs_path.filename().string() + ".converted.csv")};
    return output_path;
}

Graphite::Settings::PersistentSettings RegexTextV1LogsPlugin::GetConfig() const
{
    return Graphite::Settings::PersistentSettings{m_home_path, "config"};
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin()
{
    return new Fluxion::Plugins::Logs::RegexTextV1::RegexTextV1LogsPlugin();
}
