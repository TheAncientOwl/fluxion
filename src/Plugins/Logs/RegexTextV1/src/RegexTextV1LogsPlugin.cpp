/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTextV1LogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.10
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <string>

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

std::filesystem::path RegexTextV1LogsPlugin::MakeConvertedLogsPath(
    std::filesystem::path const& raw_logs_path) const
{
    auto const output_path{m_home_path / (raw_logs_path.filename().string() + ".converted.csv")};
    return output_path;
}

std::filesystem::path RegexTextV1LogsPlugin::MakeFilteredLogsPath(
    std::filesystem::path const& raw_logs_path) const
{
    auto const output_path{m_home_path / (raw_logs_path.filename().string() + ".filtered.csv")};
    return output_path;
}

Graphite::Settings::PersistentSettings RegexTextV1LogsPlugin::GetConfig() const
{
    return Graphite::Settings::PersistentSettings{m_home_path, "config"};
}

void RegexTextV1LogsPlugin::SaveRegexTags(std::vector<std::shared_ptr<Data::RegexTag>> const& tags) const
{
    LOG_TRACE("::SaveRegexTags()");
    LOG_INFO("::SaveRegexTags(): size == {}", tags.size());
    auto tags_json = nlohmann::json::array();

    for (auto const& tag : tags)
    {
        tags_json.push_back(
            nlohmann::json{
                {"id", tag->id.ToString()},
                {"display_name", tag->display_name},
                {"regex_data", tag->regex_data},
                {"visible", tag->visible}});
    }

    auto config{GetConfig()};
    config.SetJsonValue("tags", tags_json);
    config.Save();
}

std::vector<std::shared_ptr<Data::RegexTag>> RegexTextV1LogsPlugin::LoadRegexTags() const
{
    LOG_TRACE("::LoadRegexTags()");
    auto config{GetConfig()};
    auto tags_json_opt = config.GetJsonValue("tags");

    if (!tags_json_opt || !tags_json_opt->is_array())
    {
        LOG_WARN("::LoadRegexTags(): No tags found or invalid format.");
        return {};
    }

    RegexTags loaded_tags;

    for (const auto& tag_obj : tags_json_opt.value())
    {
        try
        {
            auto tag = std::make_shared<Data::RegexTag>();
            tag->id = Graphite::Common::Utility::UniqueID(tag_obj.at("id").get<std::string>());
            tag->display_name = tag_obj.at("display_name").get<std::string>();
            tag->regex_data = tag_obj.at("regex_data").get<std::string>();
            tag->visible = tag_obj.at("visible").get<bool>();

            loaded_tags.push_back(tag);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("::LoadRegexTags(): Failed to parse tag: {}", e.what());
        }
    }

    LOG_INFO("::SaveRegexTags(): size == {}", loaded_tags.size());
    return loaded_tags;
}

void RegexTextV1LogsPlugin::UpdateImportedLogsHeader(
    std::vector<std::shared_ptr<Data::RegexTag>> const& tags)
{
    LOG_TRACE("::UpdateImportedLogsHeader():");
    LOG_INFO("::UpdateImportedLogsHeader(): tags size == {}", tags.size());

    m_imported_logs_header.clear();
    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            m_imported_logs_header.push_back({tag->id, tag->display_name});
        }
    }
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin()
{
    return new Fluxion::Plugins::Logs::RegexTextV1::RegexTextV1LogsPlugin();
}
