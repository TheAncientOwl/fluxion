/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 3.5
/// @brief Implementation @see RegexTags.hpp
///

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <re2/re2.h>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V3/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "SQLite/BufferedWriter.hpp"
#include "SQLite/Creator.hpp"
#include "SQLite/Utility.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::ImportLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3 {

namespace Utility {

size_t CountLines(const std::filesystem::path& filepath)
{
    LOG_SCOPE("::CountLines()");
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

    m_logs_operation_progress = 0;
    m_logs_operation_target = Utility::CountLines(path);

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

    auto line_regex = std::make_unique<re2::RE2>(full_pattern);
    if (!line_regex->ok())
    {
        LOG_WARN("Invalid regex: {}", line_regex->error());
        return;
    }

    m_last_imported_logs_path = path;
    std::ifstream raw_logs_file{path};
    if (!raw_logs_file.is_open())
    {
        LOG_WARN("::ImportLogs(): Could not open file {}", path.c_str());
        return;
    }

    auto const database_path{MakeDatabasePath(path)};
    [[maybe_unused]] std::error_code ec{};
    std::filesystem::remove(database_path);
    std::filesystem::remove(database_path.string() + "-wal", ec);
    std::filesystem::remove(database_path.string() + "-shm", ec);
    if (!m_sqlite_connection.OpenDatabase(database_path))
    {
        return;
    }

    auto const fields_ids{SQLite::Utility::MakeFieldsIDs(tags)};

    if (!SQLite::Creator{m_sqlite_connection.GetDatabaseRef()}.CreateTables(fields_ids))
    {
        return;
    }

    auto const default_filter_id{Graphite::Common::Utility::UniqueID::Default().ToString()};

    std::string line{};
    line.reserve(1024);

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

    int const num_captures_int = line_regex->NumberOfCapturingGroups();
    std::size_t const num_captures = static_cast<std::size_t>(num_captures_int);

    std::vector<std::string> capture_results(num_captures);
    std::vector<re2::RE2::Arg> re2_args;
    std::vector<re2::RE2::Arg*> re2_arg_ptrs;
    re2_args.reserve(num_captures);
    re2_arg_ptrs.reserve(num_captures);

    for (std::size_t i = 0; i < num_captures; ++i)
    {
        re2_args.emplace_back(&capture_results[i]);
        re2_arg_ptrs.push_back(&re2_args.back());
    }

    {
        LOG_SCOPE("::ImportLogs(): writer");
        auto sqlite_writer{
            SQLite::BufferedWriter{m_sqlite_connection.GetDatabaseRef(), 1000, fields_ids}};

        while (std::getline(raw_logs_file, line))
        {
            bool const matched =
                re2::RE2::FullMatchN(line, *line_regex, re2_arg_ptrs.data(), num_captures_int);

            if (matched)
            {
                auto& next_row{sqlite_writer.NextFrame()};

                ++m_logs_operation_progress;
                if (m_logs_operation_progress % 1000 == 0)
                {
                    LOG_INFO("Read another 1000 chunk, total: {}", m_logs_operation_progress);
                }

                for (std::size_t i = 0; i < num_captures && i < row.size(); ++i)
                {
                    next_row[i] = capture_results[i];
                }
            }
            else
            {
                LOG_WARN("::ImportLogs(): Regex did not match entry '{}'", line);
            }
        }

        sqlite_writer.Flush();
    }

    LOG_INFO("::ImportLogs(): Total matched logs: {}", m_logs_operation_progress);
    auto settings{GetConfig()};
    settings.set("total_logs", m_logs_operation_progress);
    settings.set("total_logs_imported", m_logs_operation_progress);
    settings.Save();

    m_logs_operation_target = 0;
    m_logs_operation_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3
