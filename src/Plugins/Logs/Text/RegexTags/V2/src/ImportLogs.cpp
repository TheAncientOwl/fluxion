/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation @see RegexTags.hpp
///

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V2/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V2 {

namespace Utility {

size_t CountLines(const std::filesystem::path& filepath)
{
    std::FILE* file = std::fopen(filepath.string().c_str(), "rb");
    if (!file)
        return 0;

    size_t lines = 0;
    constexpr size_t buffer_size = 65536; // 64 KB buffer
    char buffer[buffer_size];

    size_t bytes_read = 0;
    while ((bytes_read = std::fread(buffer, 1, buffer_size, file)) > 0)
    {
        const char* ptr = buffer;
        const char* buffer_end = buffer + bytes_read;

        while ((ptr = static_cast<const char*>(
                    std::memchr(ptr, '\n', static_cast<size_t>(buffer_end - ptr)))) != nullptr)
        {
            ++lines;
            ++ptr; // Move past the newline character
        }
    }

    std::fclose(file);
    return lines;
}

} // namespace Utility

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
    m_logs_progress = 0;
    m_total_import_logs = Utility::CountLines(path);

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
            ++m_logs_progress;
            if (m_logs_progress % 1000 == 0)
            {
                LOG_INFO("Read another 1000 chunk, total: {}", m_logs_progress);
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

    LOG_INFO("::ImportLogs(): Total matched logs: {}", m_logs_progress);
    auto settings{GetConfig()};
    settings.set("total_logs", m_logs_progress);
    settings.set("total_logs_imported", m_logs_progress);
    settings.Save();

    m_total_import_logs = 0;
    m_logs_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V2
