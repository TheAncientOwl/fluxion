/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file PluginTest.cpp
/// @author Alexandru Delegeanu
/// @version 7.1
/// @brief Logs::Text::RegexTags::V7 Google Test Suite
///

#include <fstream>
#include <gtest/gtest.h>

#include "Fluxion/API/testing/LogsPluginTestingToolkit.hpp"
#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"

using namespace Fluxion::API::Testing::LogsPluginTestingKit;

class LogsPluginWrapper : public ILogsPluginTestWrapper
{
public:
    /**
     * @brief Create logs plugin and setup its internals
     */
    void Setup(std::filesystem::path const& plugin_home_path) final override
    {
        m_home_path = plugin_home_path;
        m_raw_logs_file.open(GetImportLogsFilePath());

        if (!m_raw_logs_file.is_open())
        {
            throw std::runtime_error(
                "Failed to open log file for writing at: " + GetImportLogsFilePath().string());
        }

        {
            // override tags to match generated test entries style
            std::vector<Fluxion::Plugins::Logs::Text::RegexTags::V7::Data::RegexTag> tags{};
            {
                auto& new_tag = tags.emplace_back();
                new_tag.display_name = "Timestamp";
                new_tag.regex_data = R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d+)";
                new_tag.id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag.visible = true;
            }
            {
                auto& new_tag = tags.emplace_back();
                new_tag.display_name = "-";
                new_tag.regex_data = R"(\s+)";
                new_tag.id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag.visible = false;
            }
            {
                auto& new_tag = tags.emplace_back();
                new_tag.display_name = "Channel";
                new_tag.regex_data = R"(Channel[1-4])";
                new_tag.id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag.visible = true;
            }
            {
                auto& new_tag = tags.emplace_back();
                new_tag.display_name = "-";
                new_tag.regex_data = R"(\s+)";
                new_tag.id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag.visible = false;
            }
            {
                auto& new_tag = tags.emplace_back();
                new_tag.display_name = "Level";
                new_tag.regex_data = R"(trace|info|error|debug|warn)";
                new_tag.id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag.visible = true;
            }
            {
                auto& new_tag = tags.emplace_back();
                new_tag.display_name = "-";
                new_tag.regex_data = R"(\s+)";
                new_tag.id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag.visible = false;
            }
            {
                auto& new_tag = tags.emplace_back();
                new_tag.display_name = "Payload";
                new_tag.regex_data = R"(.*)";
                new_tag.id = Graphite::Common::Utility::UniqueID::Generate();
                new_tag.visible = true;
            }

            auto tags_json = nlohmann::json::array();

            for (auto const& tag : tags)
            {
                tags_json.push_back(
                    nlohmann::json{
                        {"id", tag.id.ToString()},
                        {"display_name", tag.display_name},
                        {"regex_data", tag.regex_data},
                        {"visible", tag.visible}});
            }

            auto config{Graphite::Settings::PersistentSettings{m_home_path, "config"}};
            config.SetJsonValue("tags", tags_json);
            config.Save();

            m_plugin.OnEnable({.plugin_home_path = m_home_path});
        }
    }

    /**
     * @brief Cleanup
     */
    void Teardown() final override {}

    /**
     * @brief Used to generate input data file during @see LogsPluginTestSuite::Setup
     *
     * @param header
     * @param entry
     */
    void WriteLogEntryToImportFile(LogEntry::Data const& /*header*/, LogEntry::Data const& entry) final override
    {
        for (std::size_t idx = 0; idx < entry.size(); ++idx)
        {
            m_raw_logs_file << entry[idx] << (entry.size() >= 1 && idx < entry.size() - 1 ? " " : "");
        }
        m_raw_logs_file << '\n';
    }

    /**
     * @brief Cleanup after @see LogsPluginTestSuite::WriteLogEntryToImportFile
     *
     * @param header
     * @param entry
     */
    void OnLogsGenerationProcessDone() final override { m_raw_logs_file.close(); }

    /**
     * @brief Get the Logs Plugin object
     *
     * @return Fluxion::API::LogsPlugin::IFluxionLogsPlugin&
     */
    Fluxion::API::LogsPlugin::IFluxionLogsPlugin& GetLogsPlugin() final override
    {
        return m_plugin;
    }

    /**
     * @brief Get the path to the raw format log file ~ .txt
     *
     * @return std::filesystem::path
     */
    std::filesystem::path GetImportLogsFilePath() const final override
    {
        return m_home_path / "raw_logs.txt";
    }

private:
    Fluxion::Plugins::Logs::Text::RegexTags::V7::RegexTags m_plugin{};
    std::filesystem::path m_home_path{};
    std::ofstream m_raw_logs_file{};
};

FLUXION_DEFINE_LOGS_PLUGIN_TESTS(LogsPluginWrapper, {.logs_count = 2000, .seed = 69420});
