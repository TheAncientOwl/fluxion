/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Implementation @see RegexTags.hpp
///

#include <fstream>
#include <regex>
#include <string>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

void RegexTags::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("::ImportLogs()");
    LOG_INFO("Importing {}", path.c_str());

    m_regex_tags.SyncFrontBufferCopy();
    auto const& tags{m_regex_tags.GetFront()};

    std::string full_pattern{};
    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            m_imported_logs_header.push_back({tag->id, tag->display_name});
            full_pattern += "(" + tag->regex_data + ")";
        }
        else
        {
            full_pattern += tag->regex_data;
        }
    }
    UpdateImportedLogsHeader(tags);
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

    auto const output_converted_path{MakeConvertedLogsPath(path)};
    auto converted_writer = CSV::Writer{output_converted_path};
    LOG_INFO("Output converted CSV file {}", output_converted_path.string());

    auto const output_filtered_path{MakeFilteredLogsPath(path)};
    auto filtered_writer = CSV::Writer{output_filtered_path};
    LOG_INFO("Output filtered CSV file {}", output_filtered_path.string());
    auto const default_filter_id{Graphite::Common::Utility::UniqueID::Default().ToString()};

    std::string line{};
    line.reserve(1024);
    std::size_t total_logs{0};

    std::vector<std::string> row{};
    std::vector<std::string> filtered_row{default_filter_id, default_filter_id};
    std::string dummy{};
    row.reserve(m_imported_logs_header.size());
    filtered_row.reserve(m_imported_logs_header.size() + 2);
    for (auto const& _ : m_imported_logs_header)
    {
        row.push_back(dummy);
        filtered_row.push_back(dummy);
    }

    filtered_row[0] = default_filter_id;
    filtered_row[1] = default_filter_id;
    while (std::getline(raw_logs_file, line))
    {
        std::smatch matches;
        if (std::regex_match(line, matches, line_regex))
        {
            ++total_logs;
            if (total_logs % 1000 == 0)
            {
                LOG_INFO("Read another 1000 chunk, total: {}", total_logs);
            }

            // matches[0] is full match, start from 1
            for (std::size_t match_idx = 1; match_idx < matches.size(); ++match_idx)
            {
                if (matches[match_idx].matched)
                {
                    auto match{matches[match_idx].str()};
                    row[match_idx - 1] = match;
                    filtered_row[match_idx + 1] = std::move(match);
                }
            }
            converted_writer.write_row(row);
            filtered_writer.write_row(filtered_row);
        }
        else
        {
            LOG_WARN("::ImportLogs(): Regex did not match entry '{}'", line);
        }
    }

    LOG_INFO("::ImportLogs(): Total matched logs: {}", total_logs);
    auto settings{GetConfig()};
    settings.set("total_logs", total_logs);
    settings.Save();
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
