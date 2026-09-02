/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation @see RegexTags.hpp
///

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <re2/re2.h>
#include <re2/stringpiece.h>
#include <sqlite3.h>
#include <string>
#include <string_view>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <thread>
#include <unordered_map>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::ImportLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::ImportLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

namespace Utility {

class LogsOperationUnitResetter
{
public:
    LogsOperationUnitResetter(Fluxion::API::LogsPlugin::Data::ELogsOperationUnit& target)
        : m_target{target}
    {
    }

    ~LogsOperationUnitResetter()
    {
        m_target = Fluxion::API::LogsPlugin::Data::ELogsOperationUnit::Logs;
    };

private:
    Fluxion::API::LogsPlugin::Data::ELogsOperationUnit& m_target;
};

struct MappedFile
{
    struct Deleter
    {
        std::size_t size{0};

        void operator()(const char* ptr) const
        {
            if (ptr)
            {
#if defined(_WIN32)
                ::UnmapViewOfFile(static_cast<LPCVOID>(ptr));
#else
                if (ptr != MAP_FAILED)
                {
                    ::munmap(const_cast<char*>(ptr), size);
                }
#endif
            }
        }
    };

    std::unique_ptr<const char, Deleter> data{nullptr, Deleter{}};
    std::size_t size{0};

    [[nodiscard]] bool IsValid() const
    {
#if defined(_WIN32)
        return data != nullptr && size > 0;
#else
        return data != nullptr && data.get() != MAP_FAILED && size > 0;
#endif
    }

    [[nodiscard]] const char* get() const { return data.get(); }
};

MappedFile MapFile(std::filesystem::path const& path)
{
    LOG_SCOPE("::MapFile()");

#if defined(_WIN32)
    HANDLE hFile = ::CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return {};
    }

    LARGE_INTEGER file_size_li;
    if (!::GetFileSizeEx(hFile, &file_size_li) || file_size_li.QuadPart == 0)
    {
        ::CloseHandle(hFile);
        return {};
    }

    auto const file_size = static_cast<std::size_t>(file_size_li.QuadPart);

    HANDLE hMapping = ::CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);

    ::CloseHandle(hFile);

    if (!hMapping)
    {
        return {};
    }

    void* mapped_ptr = ::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);

    ::CloseHandle(hMapping);

    if (!mapped_ptr)
    {
        return {};
    }

    // Windows equivalent of madvise(..., MADV_WILLNEED/SEQUENTIAL)
    WIN32_MEMORY_RANGE_ENTRY rangeEntry;
    rangeEntry.VirtualAddress = mapped_ptr;
    rangeEntry.NumberOfBytes = file_size;
    ::PrefetchVirtualMemory(::GetCurrentProcess(), 1, &rangeEntry, 0);

    return MappedFile{
        .data = std::unique_ptr<const char, MappedFile::Deleter>(
            static_cast<const char*>(mapped_ptr), MappedFile::Deleter{file_size}),
        .size = file_size};

#else
    int const fd = ::open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        return {};
    }

    struct stat sb;
    if (::fstat(fd, &sb) == -1 || sb.st_size == 0)
    {
        ::close(fd);
        return {};
    }

    auto const file_size = static_cast<std::size_t>(sb.st_size);

    void* mapped_ptr = ::mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd);

    if (mapped_ptr == MAP_FAILED)
    {
        return {};
    }

    ::madvise(mapped_ptr, file_size, MADV_SEQUENTIAL);

    return MappedFile{
        .data = std::unique_ptr<const char, MappedFile::Deleter>(
            static_cast<const char*>(mapped_ptr), MappedFile::Deleter{file_size}),
        .size = file_size};
#endif
}

namespace Multithreading {

struct FileSlice
{
    std::size_t slice_id{0};
    const char* start{nullptr};
    const char* end{nullptr};
};

inline std::vector<FileSlice> SplitFile(
    const char* const file_data,
    std::size_t const file_size,
    std::size_t const target_slice_bytes = 4 * 1024 * 1024,
    char const target_char = '\n')
{
    LOG_SCOPE("::SplitFile()");
    std::vector<FileSlice> slices{};
    if (file_size == 0)
    {
        return slices;
    }

    const char* current_ptr{file_data};
    const char* const file_end{file_data + file_size};
    std::size_t slice_id{0};

    while (current_ptr < file_end)
    {
        const char* target{current_ptr + target_slice_bytes};
        const char* end_ptr{file_end};

        if (target < file_end)
        {
            const char* newline{static_cast<const char*>(
                std::memchr(target, target_char, static_cast<std::size_t>(file_end - target)))};
            end_ptr = newline ? newline + 1 : file_end;
        }

        slices.push_back(FileSlice{slice_id++, current_ptr, end_ptr});
        current_ptr = end_ptr;
    }
    return slices;
}

struct LogChunk
{
    std::size_t slice_id{0};
    std::size_t chunk_index{0}; // Local sequence within slice
    std::vector<std::vector<std::string_view>> rows;
    std::size_t active_populated_rows{0};
    std::size_t chunk_size_bytes{0};

    LogChunk(std::size_t const capacity, std::size_t const field_count)
        : rows(capacity, std::vector<std::string_view>(field_count))
    {
    }
};

class DynamicChunkQueue
{
public:
    DynamicChunkQueue(
        std::size_t const total_slices,
        std::size_t const total_chunks,
        std::size_t const capacity_per_chunk,
        std::size_t const field_count)
        : m_ready_chunks(total_slices), m_slice_done(total_slices, false)
    {
        LOG_SCOPE("::DynamicChunkQueue()");
        m_free_pool.reserve(total_chunks);
        for (std::size_t chunk_idx = 0; chunk_idx < total_chunks; ++chunk_idx)
        {
            m_free_pool.emplace_back(std::make_unique<LogChunk>(capacity_per_chunk, field_count));
        }
    }

    std::unique_ptr<LogChunk> AcquireFreeChunk(std::size_t const slice_id)
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_cv_free.wait(lock, [this] { return !m_free_pool.empty(); });
        auto chunk = std::move(m_free_pool.back());
        m_free_pool.pop_back();

        chunk->slice_id = slice_id;
        chunk->active_populated_rows = 0;
        chunk->chunk_size_bytes = 0;
        return chunk;
    }

    void SubmitFilledChunk(std::unique_ptr<LogChunk> chunk)
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        std::size_t const s_id = chunk->slice_id;
        std::size_t const c_idx = chunk->chunk_index;
        m_ready_chunks[s_id].emplace(c_idx, std::move(chunk));
        m_cv_writer.notify_one();
    }

    void MarkSliceDone(std::size_t const slice_id)
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_slice_done[slice_id] = true;
        m_cv_writer.notify_all();
    }

    std::unique_ptr<LogChunk> PopNextChunk(std::size_t const slice_id, std::size_t const expected_index)
    {
        std::unique_lock<std::mutex> lock{m_mutex};

        m_cv_writer.wait(lock, [&] {
            return m_ready_chunks[slice_id].contains(expected_index) || m_slice_done[slice_id];
        });

        auto& slice_map = m_ready_chunks[slice_id];
        if (auto const it = slice_map.find(expected_index); it != slice_map.end())
        {
            auto chunk = std::move(it->second);
            slice_map.erase(it);
            return chunk;
        }

        return nullptr;
    }

    void RecycleChunk(std::unique_ptr<LogChunk> chunk)
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_free_pool.push_back(std::move(chunk));
        m_cv_free.notify_one();
    }

private:
    std::mutex m_mutex{};
    std::condition_variable m_cv_free{};
    std::condition_variable m_cv_writer{};

    std::vector<std::unique_ptr<LogChunk>> m_free_pool{};
    std::vector<std::unordered_map<std::size_t, std::unique_ptr<LogChunk>>> m_ready_chunks{};
    std::vector<bool> m_slice_done{};
};

} // namespace Multithreading

} // namespace Utility

void RegexTags::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("::ImportLogs()");
    LOG_INFO("Importing {}", path);

    m_filtered_logs.clear();
    m_total_logs_imported = 0;

    m_regex_tags.SyncFrontBufferCopy();
    auto const tags{m_regex_tags.GetFront()};
    std::string line_regex_pattern{};
    {
        LOG_SCOPE("::LineRegexPatternMaker()");
        for (auto const& tag : tags)
        {
            if (tag->visible)
            {
                line_regex_pattern += "(" + tag->regex_data + ")";
            }
            else
            {
                line_regex_pattern += tag->regex_data;
            }
        }
        LOG_INFO("::ImportLogs(): Full regex pattern: {}", line_regex_pattern);
    }

    re2::RE2 const shared_regex(line_regex_pattern);
    if (!shared_regex.ok())
    {
        LOG_WARN("Invalid regex: {}", shared_regex.error());
        return;
    }

    UpdateImportedLogsHeader(tags);

    auto mapped_file = Utility::MapFile(path);
    if (!mapped_file.IsValid())
    {
        LOG_ERROR("::ImportLogs(): Failed to map file or file is empty: {}", path);
        return;
    }

    m_last_imported_logs_path = path;
    m_logs_operation_progress = 0;
    Utility::LogsOperationUnitResetter logs_operation_unit_resetter{m_logs_operation_unit};
    m_logs_operation_unit = Fluxion::API::LogsPlugin::Data::ELogsOperationUnit::Bytes;
    m_logs_operation_target = mapped_file.size;

    {
        LOG_SCOPE("::CreateDatabase()");
        m_scrolls.Close();
        auto const database_path{MakeDatabasePath(path)};
        [[maybe_unused]] std::error_code ec{};
        std::filesystem::remove_all(database_path, ec);
        if (auto const status = m_scrolls.OpenWrite(
                database_path, 1, mapped_file.size, m_imported_logs_header.size());
            status != Scrolls::Papyrus::EWriteStatus::Success)
        {
            LOG_ERROR(
                "::ImportLogs(): failed to open scrolls, status {}",
                static_cast<std::uint64_t>(status));
            return;
        }
    }

    auto const row_fields_count{m_imported_logs_header.size()};

    auto const file_slices = Utility::Multithreading::SplitFile(
        mapped_file.get(),
        mapped_file.size,
        static_cast<std::size_t>(m_settings.import_params.file_target_slice_mb) * 1024 * 1024);
    auto const total_slices = file_slices.size();

    Utility::Multithreading::DynamicChunkQueue queue(
        total_slices,
        static_cast<std::size_t>(
            m_settings.import_params.workers_count *
            m_settings.import_params.available_batches_per_worker),
        static_cast<std::size_t>(m_settings.import_params.batch_capacity),
        row_fields_count);

    // 1. Single Writer Thread: Consumes chunks sequentially per slice index
    auto writer_future = std::async(std::launch::async, [&]() {
        LOG_SCOPE("::ImportLogs(): writer_thread");

        auto scroll_writers{m_scrolls.GetWriters()};
        auto& scroll_writer{scroll_writers.front()};

        std::size_t idx{0};

        for (std::size_t slice_idx = 0; slice_idx < total_slices; ++slice_idx)
        {
            std::size_t expected_chunk_idx{0};
            while (true)
            {
                auto chunk = queue.PopNextChunk(slice_idx, expected_chunk_idx);
                if (!chunk)
                {
                    break;
                }

                for (std::size_t row_idx = 0; row_idx < chunk->active_populated_rows; ++row_idx)
                {
                    std::ignore = scroll_writer.Write(chunk->rows[row_idx]);
                    m_filtered_logs.emplace_back(idx++);
                }

                m_logs_operation_progress += chunk->chunk_size_bytes;

                queue.RecycleChunk(std::move(chunk));
                ++expected_chunk_idx;
            }
        }
    });

    // 2. Parallel Worker Threads using shared RE2 instance
    std::vector<std::thread> workers{};
    {
        LOG_SCOPE("::ParserWorkerThreadsCreation()");
        workers.reserve(static_cast<std::size_t>(m_settings.import_params.workers_count));
        std::atomic<std::size_t> next_slice_idx{0};
        auto const num_captures = static_cast<std::size_t>(shared_regex.NumberOfCapturingGroups());

        for (unsigned int worker_idx = 0;
             worker_idx < static_cast<std::size_t>(m_settings.import_params.workers_count);
             ++worker_idx)
        {
            workers.emplace_back([&,
                                  num_captures,
                                  batch_capacity = m_settings.import_params.batch_capacity]() {
                LOG_SCOPE("::ParserWorkerThread::{}()", std::this_thread::get_id());

                // Per-thread capture buffer reuse
                std::vector<re2::StringPiece> capture_results(num_captures);
                std::vector<re2::RE2::Arg> re2_args{};
                std::vector<re2::RE2::Arg*> re2_arg_ptrs{};
                re2_args.reserve(num_captures);
                re2_arg_ptrs.reserve(num_captures);

                for (std::size_t capture_idx = 0; capture_idx < num_captures; ++capture_idx)
                {
                    re2_args.emplace_back(&capture_results[capture_idx]);
                    re2_arg_ptrs.push_back(&re2_args.back());
                }

                while (true)
                {
                    auto const slice_idx{next_slice_idx.fetch_add(1, std::memory_order_relaxed)};
                    if (slice_idx >= total_slices)
                    {
                        break;
                    }

                    const char* ptr{file_slices[slice_idx].start};
                    const char* const slice_end{file_slices[slice_idx].end};

                    std::size_t local_chunk_idx{0};
                    auto current_chunk = queue.AcquireFreeChunk(slice_idx);
                    current_chunk->chunk_index = local_chunk_idx++;

                    while (ptr < slice_end)
                    {
                        const char* newline = static_cast<const char*>(
                            std::memchr(ptr, '\n', static_cast<std::size_t>(slice_end - ptr)));

                        const char* line_end = newline ? newline : slice_end;
                        const char* next_ptr = newline ? newline + 1 : slice_end;

                        current_chunk->chunk_size_bytes += static_cast<std::size_t>(next_ptr - ptr);

                        auto line_length = static_cast<std::size_t>(line_end - ptr);
                        if (line_length > 0 && ptr[line_length - 1] == '\r')
                        {
                            --line_length;
                        }

                        re2::StringPiece const line_piece(ptr, line_length);
                        if (re2::RE2::FullMatchN(
                                line_piece,
                                shared_regex,
                                re2_arg_ptrs.data(),
                                static_cast<int>(num_captures)))
                        {
                            auto& row = current_chunk->rows[current_chunk->active_populated_rows++];

                            for (std::size_t i = 0; i < num_captures && i < row_fields_count; ++i)
                            {
                                if (capture_results[i].data() != nullptr)
                                {
                                    row[i] = std::string_view(
                                        capture_results[i].data(), capture_results[i].size());
                                }
                                else
                                {
                                    row[i] = {};
                                }
                            }

                            if (current_chunk->active_populated_rows ==
                                static_cast<std::size_t>(batch_capacity))
                            {
                                queue.SubmitFilledChunk(std::move(current_chunk));
                                current_chunk = queue.AcquireFreeChunk(slice_idx);
                                current_chunk->chunk_index = local_chunk_idx++;
                            }
                        }

                        ptr = next_ptr;
                    }

                    if (current_chunk->active_populated_rows > 0 || current_chunk->chunk_size_bytes > 0)
                    {
                        queue.SubmitFilledChunk(std::move(current_chunk));
                    }
                    else
                    {
                        queue.RecycleChunk(std::move(current_chunk));
                    }

                    queue.MarkSliceDone(slice_idx);
                }
            });
        }
    }

    for (auto& worker : workers)
    {
        worker.join();
    }

    writer_future.wait();

    m_total_logs_imported = m_filtered_logs.size();
    LOG_INFO("::ImportLogs(): Total matched logs: {}", m_total_logs_imported);

    m_logs_operation_target = 0;
    m_logs_operation_progress = 0;

    std::ignore = m_scrolls.DowngradeReadOnly();
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
