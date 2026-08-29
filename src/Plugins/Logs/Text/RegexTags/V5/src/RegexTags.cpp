/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTags.cpp
/// @author Alexandru Delegeanu
/// @version 5.2
/// @brief Implementation @see RegexTags.hpp
///

#include <string>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V5/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::RegexTags);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::RegexTags);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5 {

RegexTags::RegexTags() = default;
RegexTags::~RegexTags() = default;

std::string_view RegexTags::GetDisplayName() const
{
    return "Text::RegexTags::V5";
}

std::string_view RegexTags::GetDirectoryName() const
{
    return "Text_RegexTags_V5";
}

std::filesystem::path RegexTags::MakeDatabasePath(std::filesystem::path const& raw_logs_path) const
{
    auto const output_path{m_home_path / (raw_logs_path.filename().string() + ".sqlite")};
    return output_path;
}

Graphite::Settings::PersistentSettings RegexTags::GetConfig() const
{
    return Graphite::Settings::PersistentSettings{m_home_path, "config"};
}

void RegexTags::SaveRegexTags(std::vector<std::shared_ptr<Data::RegexTag>> const& tags) const
{
    LOG_SCOPE("::SaveRegexTags()");
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

std::vector<std::shared_ptr<Data::RegexTag>> RegexTags::LoadRegexTags() const
{
    LOG_SCOPE("::LoadRegexTags()");
    auto config{GetConfig()};
    auto tags_json_opt = config.GetJsonValue("tags");

    if (!tags_json_opt || !tags_json_opt->is_array())
    {
        LOG_WARN("::LoadRegexTags(): No tags found or invalid format.");
        return {};
    }

    Data::RegexTags loaded_tags;

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

void RegexTags::SaveSettings() const
{
    LOG_SCOPE("::SaveSettings()");
    auto config{GetConfig()};
    config.SetJsonValue("import", m_settings.import_params);
    config.Save();
}

void RegexTags::LoadSettings()
{
    LOG_SCOPE("::LoadSettings()");
    auto config{GetConfig()};
    auto json_opt = config.GetJsonValue("import");

    if (!json_opt || !json_opt->is_object())
    {
        LOG_WARN("::LoadSettings(): Missing or invalid 'import' configuration.");
        m_settings.import_params = Data::Settings::ImportMultithreadingParams{};
        return;
    }

    try
    {
        m_settings.import_params = json_opt->get<Data::Settings::ImportMultithreadingParams>();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("::LoadSettings(): Failed to parse import settings: {}", e.what());
    }
}

void RegexTags::UpdateImportedLogsHeader(std::vector<std::shared_ptr<Data::RegexTag>> const& tags)
{
    LOG_SCOPE("::UpdateImportedLogsHeader():");
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

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin()
{
    return new Fluxion::Plugins::Logs::Text::RegexTags::V5::RegexTags();
}
