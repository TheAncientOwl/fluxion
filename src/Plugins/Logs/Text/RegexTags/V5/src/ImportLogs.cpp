/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 5.2
/// @brief Implementation @see RegexTags.hpp
///

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <re2/re2.h>
#include <re2/stringpiece.h>
#include <sqlite3.h>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V5/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "SQLite/Creator.hpp"
#include "SQLite/LogsWriter.hpp"
#include "SQLite/Utility.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::ImportLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::ImportLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5 {

namespace Utility {

struct MappedFile
{
    struct Deleter
    {
        std::size_t size{0};

        void operator()(const char* ptr) const
        {
            if (ptr && ptr != MAP_FAILED)
            {
                ::munmap(const_cast<char*>(ptr), size);
            }
        }
    };

    std::unique_ptr<const char, Deleter> data{nullptr};
    std::size_t size{0};

    [[nodiscard]] bool IsValid() const
    {
        return data != nullptr && data.get() != MAP_FAILED && size > 0;
    }

    [[nodiscard]] const char* get() const { return data.get(); }
};

MappedFile MapFile(std::filesystem::path const& path)
{
    LOG_SCOPE("::MapFile()");
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
    void* mapped_ptr = ::mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    ::close(fd);

    if (mapped_ptr == MAP_FAILED)
    {
        return {};
    }

    ::madvise(mapped_ptr, file_size, MADV_WILLNEED | MADV_SEQUENTIAL);

    return MappedFile{
        .data = std::unique_ptr<const char, MappedFile::Deleter>(
            static_cast<const char*>(mapped_ptr), MappedFile::Deleter{file_size}),
        .size = file_size};
}

namespace Multithreading {

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
    LOG_INFO("Importing {}", path.c_str());

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
        LOG_ERROR("::ImportLogs(): Failed to map file or file is empty: {}", path.c_str());
        return;
    }

    m_last_imported_logs_path = path;
    m_logs_progress = 0;
    m_total_import_logs =
        Utility::Multithreading::CountLinesParallel(mapped_file.get(), mapped_file.size);

    auto const fields_ids{SQLite::Utility::MakeFieldsIDs(tags)};
    {
        LOG_SCOPE("::CreateDatabase()");
        auto const database_path{MakeDatabasePath(path)};
        [[maybe_unused]] std::error_code ec{};
        std::filesystem::remove(database_path);
        std::filesystem::remove(database_path.string() + "-wal", ec);
        std::filesystem::remove(database_path.string() + "-shm", ec);
        if (!m_sqlite_connection.OpenDatabase(database_path) ||
            !SQLite::Creator{m_sqlite_connection.GetDatabaseRef()}.CreateTables(fields_ids))
        {
            return;
        }
    }

    auto const row_fields_count{m_imported_logs_header.size()};
    auto const workers_count = []() {
        auto workers = std::thread::hardware_concurrency();
        return workers == 0 ? 8 : workers;
    }();

    std::size_t const batch_capacity = 5000;
    std::size_t const total_chunks = workers_count * 10;

    auto const file_slices =
        Utility::Multithreading::SplitFile(mapped_file.get(), mapped_file.size, 4 * 1024 * 1024);
    auto const total_slices = file_slices.size();

    Utility::Multithreading::DynamicChunkQueue queue(
        total_slices, total_chunks, batch_capacity, row_fields_count);

    // 1. Single Writer Thread: Consumes chunks sequentially per slice index
    auto writer_future = std::async(std::launch::async, [&]() {
        LOG_SCOPE("::ImportLogs(): writer_thread");
        SQLite::LogsWriter sqlite_writer{m_sqlite_connection.GetDatabaseRef(), fields_ids};

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

                m_logs_progress += chunk->active_populated_rows;

                sqlite_writer.WriteChunk(chunk->rows, chunk->active_populated_rows);

                queue.RecycleChunk(std::move(chunk));
                ++expected_chunk_idx;
            }
        }
    });

    // 2. Parallel Worker Threads using shared RE2 instance
    std::vector<std::thread> workers{};
    {
        LOG_SCOPE("::ParserWorkerThreadsCreation()");
        workers.reserve(workers_count);
        std::atomic<std::size_t> next_slice_idx{0};
        auto const num_captures = static_cast<std::size_t>(shared_regex.NumberOfCapturingGroups());

        for (unsigned int worker_idx = 0; worker_idx < workers_count; ++worker_idx)
        {
            workers.emplace_back([&, num_captures]() {
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

                            if (current_chunk->active_populated_rows == batch_capacity)
                            {
                                queue.SubmitFilledChunk(std::move(current_chunk));
                                current_chunk = queue.AcquireFreeChunk(slice_idx);
                                current_chunk->chunk_index = local_chunk_idx++;
                            }
                        }

                        ptr = newline ? newline + 1 : slice_end;
                    }

                    if (current_chunk->active_populated_rows > 0)
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

    LOG_INFO("::ImportLogs(): Total matched logs: {}", m_logs_progress);

    auto settings{GetConfig()};
    settings.set("total_logs", m_logs_progress);
    settings.set("total_logs_imported", m_logs_progress);
    settings.Save();

    m_total_import_logs = 0;
    m_logs_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5
