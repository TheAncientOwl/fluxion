/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 9.8
/// @brief Implementation @see RegexTags.hpp
///

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
#include <re2/re2.h>
#include <re2/stringpiece.h>
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

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::ImportLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::ImportLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

namespace Utility {

std::string MakeLineRegexPattern(
    std::vector<std::shared_ptr<Fluxion::Plugins::Logs::Text::RegexTags::V9::Data::RegexTag>> const& tags)
{
    LOG_SCOPE("::LineRegexPatternMaker()");
    std::string out{};
    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            out += "(" + tag->regex_data + ")";
        }
        else
        {
            out += tag->regex_data;
        }
    }
    LOG_INFO("::ImportLogs(): Full regex pattern: {}", out);

    return out;
}

std::vector<std::string> MakeFields(std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> const& header)
{
    LOG_SCOPE("::MakeFields()");
    std::vector<std::string> out{};
    out.reserve(header.size());
    for (auto const& column : header)
    {
        out.push_back("field_" + column.id.ToRawString());
    }
    return out;
}

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
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
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
    const char* begin{nullptr};
    const char* end{nullptr};
};

inline std::vector<FileSlice> SplitFileSlice(
    FileSlice const& slice,
    std::size_t const target_slice_bytes = 4 * 1024 * 1024,
    char const target_char = '\n')
{
    LOG_SCOPE("::SplitFileSlice()");
    std::vector<FileSlice> slices{};
    if (!slice.begin || !slice.end || slice.begin >= slice.end)
    {
        return slices;
    }

    const char* current_ptr{slice.begin};
    const char* const file_end{slice.end};

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

        slices.push_back(FileSlice{current_ptr, end_ptr});
        current_ptr = end_ptr;
    }
    return slices;
}

struct LogChunk
{
    std::size_t shard_id{0};
    std::size_t task_id{0};
    std::size_t local_chunk_idx{0};
    bool is_last_in_task{false};

    std::vector<std::string_view> rows;
    std::size_t active_populated_rows{0};
    std::size_t chunk_size_bytes{0};
    std::size_t field_count{0};

    LogChunk(std::size_t const capacity, std::size_t const field_count)
        : rows(capacity * field_count), field_count(field_count)
    {
    }

    [[nodiscard]] inline std::string_view* GetRow(std::size_t const row_index)
    {
        return rows.data() + (row_index * field_count);
    }
};

// Shard isolated resources for lock fragmentation
struct ShardQueue
{
    std::mutex mutex{};
    std::condition_variable cv{};
    std::unordered_map<uint64_t, std::unique_ptr<LogChunk>> ready_chunks{};
};

class ChunksQueue
{
public:
    ChunksQueue(
        std::size_t const total_shards,
        std::size_t const initial_total_chunks,
        std::size_t const capacity_per_chunk,
        std::size_t const field_count)
        : m_capacity_per_chunk(capacity_per_chunk)
        , m_field_count(field_count)
        , m_max_pool_size(initial_total_chunks * 2)
    {
        LOG_SCOPE("::GlobalChunkQueue()");

        m_shards.reserve(total_shards);
        for (std::size_t i = 0; i < total_shards; ++i)
        {
            m_shards.push_back(std::make_unique<ShardQueue>());
        }

        m_free_pool.reserve(initial_total_chunks);
        for (std::size_t i = 0; i < initial_total_chunks; ++i)
        {
            m_free_pool.emplace_back(std::make_unique<LogChunk>(capacity_per_chunk, field_count));
        }
    }

    std::unique_ptr<LogChunk> Acquire()
    {
        std::unique_lock<std::mutex> lock{m_pool_mutex};
        if (!m_free_pool.empty())
        {
            auto chunk = std::move(m_free_pool.back());
            m_free_pool.pop_back();
            return chunk;
        }
        lock.unlock();

        return std::make_unique<LogChunk>(m_capacity_per_chunk, m_field_count);
    }

    void Submit(std::unique_ptr<LogChunk> chunk)
    {
        std::size_t const shard_id = chunk->shard_id;
        uint64_t const key = (static_cast<uint64_t>(chunk->task_id) << 32) | chunk->local_chunk_idx;

        auto& shard = m_shards[shard_id];
        {
            // Only block the writer thread assigned to this specific shard
            std::unique_lock<std::mutex> lock{shard->mutex};
            shard->ready_chunks[key] = std::move(chunk);
        }

        // notify_one is safe here because we have 1 dedicated writer thread per shard
        shard->cv.notify_one();
    }

    std::unique_ptr<LogChunk> Pop(std::size_t const shard_id, std::size_t const task_id, std::size_t const local_idx)
    {
        uint64_t const key = (static_cast<uint64_t>(task_id) << 32) | local_idx;
        auto& shard = m_shards[shard_id];

        std::unique_lock<std::mutex> lock{shard->mutex};
        shard->cv.wait(lock, [&] { return shard->ready_chunks.contains(key); });

        auto chunk = std::move(shard->ready_chunks[key]);
        shard->ready_chunks.erase(key);
        return chunk;
    }

    void Recycle(std::unique_ptr<LogChunk> chunk)
    {
        // Reset state lock-free to keep the critical section as tight as possible
        chunk->active_populated_rows = 0;
        chunk->chunk_size_bytes = 0;
        chunk->is_last_in_task = false;

        std::unique_lock<std::mutex> lock{m_pool_mutex};
        if (m_free_pool.size() < m_max_pool_size)
        {
            m_free_pool.push_back(std::move(chunk));
        }
    }

private:
    std::mutex m_pool_mutex{};

    std::size_t m_capacity_per_chunk{};
    std::size_t m_field_count{};
    std::size_t m_max_pool_size{};

    std::vector<std::unique_ptr<LogChunk>> m_free_pool{};
    std::vector<std::unique_ptr<ShardQueue>> m_shards{};
};

class LogsImporter
{
public:
    struct ParserTask
    {
        std::size_t shard_id{0};
        std::size_t task_id{0};
        const char* begin{nullptr};
        const char* end{nullptr};
    };

    LogsImporter(
        std::vector<FileSlice> const& mapped_file_slices,
        re2::RE2 const& shared_regex,
        std::size_t const row_fields_count,
        std::vector<std::unique_ptr<SQLiteStorage>>& sqlite_storages,
        std::size_t const workers_count,
        std::size_t const available_batches_per_worker,
        std::size_t const batch_capacity,
        std::size_t const target_slice_bytes,
        std::atomic<std::size_t>& logs_operation_progress)
        : m_shared_regex(shared_regex)
        , m_row_fields_count(row_fields_count)
        , m_sqlite_storages(sqlite_storages)
        , m_workers_count(workers_count)
        , m_batch_capacity(batch_capacity)
        , m_logs_operation_progress(logs_operation_progress)
    {
        LOG_SCOPE("::GlobalLogsImporter()");
        m_tasks_per_shard.reserve(mapped_file_slices.size());

        for (std::size_t shard_id = 0; shard_id < mapped_file_slices.size(); ++shard_id)
        {
            auto sub_slices = SplitFileSlice(mapped_file_slices[shard_id], target_slice_bytes);
            m_tasks_per_shard.push_back(sub_slices.size());

            for (std::size_t task_id = 0; task_id < sub_slices.size(); ++task_id)
            {
                m_all_tasks.push_back(
                    {shard_id, task_id, sub_slices[task_id].begin, sub_slices[task_id].end});
            }
        }

        m_initial_pool_size = m_workers_count * available_batches_per_worker;
    }

    void Run()
    {
        LOG_SCOPE("::GlobalLogsImporter::Run()");
        if (m_all_tasks.empty())
            return;

        std::size_t const total_shards = m_tasks_per_shard.size();
        ChunksQueue chunks_queue(
            total_shards, m_initial_pool_size, m_batch_capacity, m_row_fields_count);

        // 1. Launch Writers (One dedicated thread per SQLite shard)
        std::vector<std::thread> writers{};
        writers.reserve(total_shards);

        for (std::size_t shard_id = 0; shard_id < total_shards; ++shard_id)
        {
            writers.emplace_back([&, shard_id]() {
                LOG_SCOPE("::WriterThread::Shard_{}", shard_id);
                std::size_t const total_tasks = m_tasks_per_shard[shard_id];

                for (std::size_t task_id = 0; task_id < total_tasks; ++task_id)
                {
                    std::size_t local_chunk_idx = 0;
                    while (true)
                    {
                        auto chunk = chunks_queue.Pop(shard_id, task_id, local_chunk_idx);

                        if (chunk->active_populated_rows > 0)
                        {
                            if (!m_sqlite_storages[shard_id]->WriteChunkSingleWriter(
                                    chunk->rows, chunk->active_populated_rows, chunk->field_count))
                            {
                                LOG_ERROR(
                                    "::GlobalLogsImporter: Failed to write chunk to shard {}",
                                    shard_id);
                            }
                        }

                        m_logs_operation_progress.fetch_add(
                            chunk->chunk_size_bytes, std::memory_order_relaxed);

                        bool const is_last = chunk->is_last_in_task;
                        chunks_queue.Recycle(std::move(chunk));

                        if (is_last)
                            break;
                        local_chunk_idx++;
                    }
                }
            });
        }

        // 2. Launch Parsers (Fixed global thread pool consuming tasks dynamically)
        std::vector<std::thread> parsers{};
        parsers.reserve(m_workers_count);
        std::atomic<std::size_t> next_task_idx{0};
        auto const num_captures = static_cast<std::size_t>(m_shared_regex.NumberOfCapturingGroups());

        for (std::size_t worker_idx = 0; worker_idx < m_workers_count; ++worker_idx)
        {
            parsers.emplace_back([&, num_captures]() {
                LOG_SCOPE("::ParserThread::{}", std::this_thread::get_id());

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
                    std::size_t const task_idx = next_task_idx.fetch_add(1, std::memory_order_relaxed);
                    if (task_idx >= m_all_tasks.size())
                    {
                        break;
                    }

                    auto const& task = m_all_tasks[task_idx];
                    const char* ptr = task.begin;
                    const char* const slice_end = task.end;

                    std::size_t local_chunk_idx = 0;
                    auto chunk = chunks_queue.Acquire();
                    chunk->shard_id = task.shard_id;
                    chunk->task_id = task.task_id;
                    chunk->local_chunk_idx = local_chunk_idx++;

                    while (ptr < slice_end)
                    {
                        const char* newline = static_cast<const char*>(
                            std::memchr(ptr, '\n', static_cast<std::size_t>(slice_end - ptr)));

                        const char* line_end = newline ? newline : slice_end;
                        const char* next_ptr = newline ? newline + 1 : slice_end;

                        chunk->chunk_size_bytes += static_cast<std::size_t>(next_ptr - ptr);

                        auto line_length = static_cast<std::size_t>(line_end - ptr);
                        if (line_length > 0 && ptr[line_length - 1] == '\r')
                        {
                            --line_length;
                        }

                        re2::StringPiece const line_piece(ptr, line_length);
                        if (re2::RE2::FullMatchN(
                                line_piece,
                                m_shared_regex,
                                re2_arg_ptrs.data(),
                                static_cast<int>(num_captures)))
                        {
                            auto* row_base = chunk->GetRow(chunk->active_populated_rows++);

                            for (std::size_t i = 0; i < num_captures && i < m_row_fields_count; ++i)
                            {
                                if (capture_results[i].data() != nullptr)
                                {
                                    row_base[i] = std::string_view(
                                        capture_results[i].data(), capture_results[i].size());
                                }
                                else
                                {
                                    row_base[i] = {};
                                }
                            }

                            if (chunk->active_populated_rows == m_batch_capacity)
                            {
                                chunks_queue.Submit(std::move(chunk));
                                chunk = chunks_queue.Acquire();
                                chunk->shard_id = task.shard_id;
                                chunk->task_id = task.task_id;
                                chunk->local_chunk_idx = local_chunk_idx++;
                            }
                        }

                        ptr = next_ptr;
                    }

                    chunk->is_last_in_task = true;
                    chunks_queue.Submit(std::move(chunk));
                }
            });
        }

        for (auto& parser : parsers)
            parser.join();
        for (auto& writer : writers)
            writer.join();
    }

private:
    re2::RE2 const& m_shared_regex;
    std::size_t m_row_fields_count;
    std::vector<std::unique_ptr<SQLiteStorage>>& m_sqlite_storages;
    std::size_t m_workers_count;
    std::size_t m_batch_capacity;
    std::size_t m_initial_pool_size;
    std::atomic<std::size_t>& m_logs_operation_progress;

    std::vector<std::size_t> m_tasks_per_shard{};
    std::vector<ParserTask> m_all_tasks{};
};

} // namespace Multithreading
} // namespace Utility

void RegexTags::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("::ImportLogs()");
    LOG_INFO("Importing {}", path);

    m_filtered_logs = std::vector<Data::FilteredLog>{};
    m_total_logs_imported = 0;

    m_regex_tags.SyncFrontBufferCopy();
    auto const tags{m_regex_tags.GetFront()};

    re2::RE2 const shared_regex(Utility::MakeLineRegexPattern(tags));
    if (!shared_regex.ok())
    {
        LOG_ERROR("Invalid regex: {}", shared_regex.error());
        return;
    }

    UpdateImportedLogsHeader(tags);

    auto mapped_file = Utility::MapFile(path);
    if (!mapped_file.IsValid())
    {
        LOG_ERROR("::ImportLogs(): Failed to map file or file is empty: {}", path);
        return;
    }

    auto const _{Utility::LogsOperationUnitResetter{m_logs_operation_unit}};
    m_last_imported_logs_path = path;
    m_logs_operation_progress = 0;
    m_logs_operation_unit = Fluxion::API::LogsPlugin::Data::ELogsOperationUnit::Bytes;
    m_logs_operation_target = mapped_file.size;

    auto const mapped_file_slices{Utility::Multithreading::SplitFileSlice(
        Utility::Multithreading::FileSlice{mapped_file.get(), mapped_file.get() + mapped_file.size},
        static_cast<std::size_t>(m_settings.import_params.file_slice_size_mb) * 1024 * 1024)};
    LOG_INFO(
        "::ImportLogs(): Generated {} slices of {}mb",
        mapped_file_slices.size(),
        m_settings.import_params.file_slice_size_mb);

    {
        LOG_SCOPE("::ImportLogs::OpenSQLite()");
        // >> Cleanup existing storage
        m_sqlite_storages.clear();
        auto const database_path{MakeDatabasePath(path)};

        std::error_code ec{};
        std::filesystem::remove_all(database_path, ec);
        std::filesystem::create_directories(database_path, ec);
        if (ec)
        {
            LOG_ERROR("::ImportLogs(): failed to create SQLite directory {}", database_path);
            return;
        }

        // >> Create new storage
        auto const fields{Utility::MakeFields(m_imported_logs_header)};
        m_sqlite_storages.reserve(mapped_file_slices.size());
        for (std::size_t slice_idx = 0; slice_idx < mapped_file_slices.size(); ++slice_idx)
        {
            auto const shard_path = database_path / ("db" + std::to_string(slice_idx) + ".sqlite");
            auto storage = std::make_unique<SQLiteStorage>();
            if (!storage->Open(shard_path, fields, slice_idx * 1'000'000'000'000ULL) ||
                !storage->BeginTransaction())
            {
                LOG_ERROR("::ImportLogs(): failed to open SQLite shard {}", shard_path);
                return;
            }
            m_sqlite_storages.push_back(std::move(storage));
        }
    }

    {
        LOG_SCOPE("::ImportLogs()::GlobalSliceWorkers()");

        Utility::Multithreading::LogsImporter importer(
            mapped_file_slices,
            shared_regex,
            m_imported_logs_header.size(),
            m_sqlite_storages,
            static_cast<std::size_t>(m_settings.import_params.workers_count),
            static_cast<std::size_t>(m_settings.import_params.available_batches_per_worker),
            static_cast<std::size_t>(m_settings.import_params.batch_capacity),
            static_cast<std::size_t>(m_settings.import_params.file_target_slice_mb) * 1024 * 1024,
            m_logs_operation_progress);

        importer.Run();
    }

    {
        LOG_SCOPE("::ImportLogs()::CommitStorage()");
        std::vector<std::thread> commit_threads{};
        {
            LOG_SCOPE("::ImportLogs()::CommitStorage::ThreadsCreation()");
            commit_threads.reserve(m_sqlite_storages.size());
            for (auto const& storage : m_sqlite_storages)
            {
                commit_threads.emplace_back([storage_ptr = storage.get()]() {
                    LOG_SCOPE("::ImportLogs()::CommitStorage::Thread()");
                    if (storage_ptr)
                    {
                        std::ignore = storage_ptr->Commit();
                    }
                });
            }
        }
        for (auto& commit : commit_threads)
        {
            commit.join();
        }
    }

    {
        LOG_SCOPE("::ImportLogs()::BuildFilteredLogsIndex()");
        std::size_t written_rows_total{0};
        for (auto const& storage : m_sqlite_storages)
        {
            written_rows_total += storage->GetWrittenRows();
        }
        m_filtered_logs.reserve(written_rows_total);
        for (auto const& storage : m_sqlite_storages)
        {
            auto const count = storage->GetWrittenRows();
            auto const offset = storage->GetIDOffset();
            for (std::size_t index = 0; index < count; ++index)
            {
                m_filtered_logs.emplace_back(offset + index);
            }
        }
    }

    m_total_logs_imported = m_filtered_logs.size();
    LOG_INFO("::ImportLogs(): Total matched logs: {}", m_total_logs_imported);

    m_logs_operation_target = 0;
    m_logs_operation_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
