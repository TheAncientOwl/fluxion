/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 4.4
/// @brief Implementation @see RegexTags.hpp
///

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <future>
#include <memory>
#include <re2/re2.h>
#include <re2/stringpiece.h>
#include <sqlite3.h>
#include <string>
#include <thread>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "Fluxion/Plugins/Logs/Text/RegexTags/V4/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "SQLite/BufferedWriter.hpp"
#include "SQLite/Creator.hpp"
#include "SQLite/Utility.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V4::ImportLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V4 {

namespace Utility {

std::size_t CountLinesParallel(const char* data, std::size_t size)
{
    LOG_SCOPE("::CountLinesParallel()");

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0)
    {
        num_threads = 8;
    }

    std::size_t const chunk_size = size / num_threads;
    std::vector<std::future<std::size_t>> futures;
    futures.reserve(num_threads);

    for (unsigned int i = 0; i < num_threads; ++i)
    {
        std::size_t const start = i * chunk_size;
        std::size_t const end = (i == num_threads - 1) ? size : (i + 1) * chunk_size;

        futures.push_back(std::async(std::launch::async, [data, start, end]() -> std::size_t {
            std::size_t lines = 0;
            const char* ptr = data + start;
            const char* const chunk_end = data + end;

            while ((ptr = static_cast<const char*>(std::memchr(
                        ptr, '\n', static_cast<std::size_t>(chunk_end - ptr)))) != nullptr)
            {
                ++lines;
                ++ptr;
            }
            return lines;
        }));
    }

    std::size_t total_lines = 0;
    for (auto& f : futures)
    {
        total_lines += f.get();
    }

    return total_lines;
}

} // namespace Utility

void RegexTags::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("::ImportLogs()");
    LOG_INFO("Importing {}", path);

    m_last_imported_logs_path = path;

    const char* file_data = nullptr;
    std::size_t file_size = 0;

#if defined(_WIN32)
    HANDLE hFile = ::CreateFileW(
        path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("::ImportLogs(): Failed to open file handle: {}", path);
        return;
    }

    LARGE_INTEGER fileSizeLi;
    if (!::GetFileSizeEx(hFile, &fileSizeLi) || fileSizeLi.QuadPart == 0)
    {
        ::CloseHandle(hFile);
        LOG_ERROR("::ImportLogs(): Empty file or size query failure: {}", path);
        return;
    }
    file_size = static_cast<std::size_t>(fileSizeLi.QuadPart);

    HANDLE hMapping = ::CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    ::CloseHandle(hFile); // Mapping maintains reference
    if (!hMapping)
    {
        LOG_ERROR("::ImportLogs(): CreateFileMapping failed for path: {}", path);
        return;
    }

    void* mapped_ptr = ::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    ::CloseHandle(hMapping); // View maintains reference

    if (!mapped_ptr)
    {
        LOG_ERROR("::ImportLogs(): MapViewOfFile failed for path: {}", path);
        return;
    }
    file_data = static_cast<const char*>(mapped_ptr);
#else
    int const fd = ::open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        LOG_ERROR("::ImportLogs(): Failed to open log file descriptor: {}", path);
        return;
    }

    struct stat sb;
    if (::fstat(fd, &sb) == -1 || sb.st_size == 0)
    {
        ::close(fd);
        LOG_ERROR("::ImportLogs(): Empty file or stat failure: {}", path);
        return;
    }

    file_size = static_cast<std::size_t>(sb.st_size);
    void* mapped_ptr = ::mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    ::close(fd);

    if (mapped_ptr == MAP_FAILED)
    {
        LOG_ERROR("::ImportLogs(): mmap failed for path: {}", path);
        return;
    }

    file_data = static_cast<const char*>(mapped_ptr);
    ::madvise(mapped_ptr, file_size, MADV_WILLNEED | MADV_SEQUENTIAL);
#endif

    m_logs_operation_progress = 0;
    m_logs_operation_target = Utility::CountLinesParallel(file_data, file_size);

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
#if defined(_WIN32)
        ::UnmapViewOfFile(mapped_ptr);
#else
        ::munmap(mapped_ptr, file_size);
#endif
        return;
    }

    m_sqlite_connection.Close();
    auto const database_path{MakeDatabasePath(path)};
    [[maybe_unused]] std::error_code ec{};
    std::filesystem::remove(database_path);
    std::filesystem::remove(database_path.string() + "-wal", ec);
    std::filesystem::remove(database_path.string() + "-shm", ec);

    if (!m_sqlite_connection.OpenDatabase(database_path))
    {
#if defined(_WIN32)
        ::UnmapViewOfFile(mapped_ptr);
#else
        ::munmap(mapped_ptr, file_size);
#endif
        return;
    }

    auto const fields_ids{SQLite::Utility::MakeFieldsIDs(tags)};
    if (!SQLite::Creator{m_sqlite_connection.GetDatabaseRef()}.CreateTables(fields_ids))
    {
#if defined(_WIN32)
        ::UnmapViewOfFile(mapped_ptr);
#else
        ::munmap(mapped_ptr, file_size);
#endif
        return;
    }

    int const num_captures_int = line_regex->NumberOfCapturingGroups();
    std::size_t const num_captures = static_cast<std::size_t>(num_captures_int);
    std::size_t const max_header_fields = m_imported_logs_header.size();

    std::vector<re2::StringPiece> capture_results(num_captures);
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
            SQLite::BufferedWriter{m_sqlite_connection.GetDatabaseRef(), 5000, fields_ids}};

        const char* ptr = file_data;
        const char* const file_end = file_data + file_size;

        while (ptr < file_end)
        {
            const char* newline = static_cast<const char*>(
                std::memchr(ptr, '\n', static_cast<std::size_t>(file_end - ptr)));
            const char* line_end = newline ? newline : file_end;

            std::size_t line_len = static_cast<std::size_t>(line_end - ptr);
            if (line_len > 0 && ptr[line_len - 1] == '\r')
            {
                --line_len;
            }

            re2::StringPiece const line_piece(ptr, line_len);

            if (re2::RE2::FullMatchN(line_piece, *line_regex, re2_arg_ptrs.data(), num_captures_int))
            {
                auto& next_row{sqlite_writer.NextFrame()};
                ++m_logs_operation_progress;

                if (m_logs_operation_progress % 1000 == 0)
                {
                    LOG_INFO("Read another 1000 chunk, total: {}", m_logs_operation_progress);
                }

                for (std::size_t i = 0; i < num_captures && i < max_header_fields; ++i)
                {
                    if (capture_results[i].data() != nullptr)
                    {
                        next_row[i].assign(capture_results[i].data(), capture_results[i].size());
                    }
                    else
                    {
                        next_row[i].clear();
                    }
                }
            }

            ptr = newline ? newline + 1 : file_end;
        }

        sqlite_writer.Flush();
    }

#if defined(_WIN32)
    ::UnmapViewOfFile(mapped_ptr);
#else
    ::munmap(mapped_ptr, file_size);
#endif

    auto const default_filter_id{Graphite::Common::Utility::UniqueID::GetDefault().ToString()};

    LOG_INFO("::ImportLogs(): Total matched logs: {}", m_logs_operation_progress);
    auto settings{GetConfig()};
    settings.set("total_logs", m_logs_operation_progress);
    settings.set("total_logs_imported", m_logs_operation_progress);
    settings.Save();

    m_logs_operation_target = 0;
    m_logs_operation_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V4
