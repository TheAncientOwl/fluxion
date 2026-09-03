/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ImportLogs.cpp
/// @author Alexandru Delegeanu
/// @version 8.0
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

#include "Fluxion/Plugins/Logs/Text/RegexTags/V8/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8::ImportLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8::ImportLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V8 {

namespace Utility {

std::string MakeLineRegexPattern(
    std::vector<std::shared_ptr<Fluxion::Plugins::Logs::Text::RegexTags::V8::Data::RegexTag>> const& tags)
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
    std::size_t slice_id{0};
    std::size_t chunk_index{0};
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
        std::size_t const initial_total_chunks,
        std::size_t const capacity_per_chunk,
        std::size_t const field_count)
        : m_capacity_per_chunk(capacity_per_chunk)
        , m_field_count(field_count)
        , m_ready_chunks(total_slices)
        , m_slice_done(total_slices, false)
    {
        LOG_SCOPE("::DynamicChunkQueue()");
        m_free_pool.reserve(initial_total_chunks);
        for (std::size_t chunk_idx = 0; chunk_idx < initial_total_chunks; ++chunk_idx)
        {
            m_free_pool.emplace_back(std::make_unique<LogChunk>(capacity_per_chunk, field_count));
        }
    }

    std::unique_ptr<LogChunk> AcquireFreeChunk(std::size_t const slice_id)
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        if (!m_free_pool.empty())
        {
            auto chunk = std::move(m_free_pool.back());
            m_free_pool.pop_back();
            lock.unlock();

            chunk->slice_id = slice_id;
            chunk->active_populated_rows = 0;
            chunk->chunk_size_bytes = 0;
            return chunk;
        }
        lock.unlock();

        // Fallback allocation: prevents pool starvation and deadlocks
        auto chunk = std::make_unique<LogChunk>(m_capacity_per_chunk, m_field_count);
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
        // TODO: Move Free pool max size to settings
        if (m_free_pool.size() < 64)
        {
            m_free_pool.push_back(std::move(chunk));
        }
    }

private:
    std::mutex m_mutex{};
    std::condition_variable m_cv_writer{};

    std::size_t m_capacity_per_chunk{};
    std::size_t m_field_count{};

    std::vector<std::unique_ptr<LogChunk>> m_free_pool{};
    std::vector<std::unordered_map<std::size_t, std::unique_ptr<LogChunk>>> m_ready_chunks{};
    std::vector<bool> m_slice_done{};
};

class LogsImporter
{
public:
    LogsImporter(
        FileSlice const& file_slice,
        re2::RE2 const& shared_regex,
        std::size_t const row_fields_count,
        SQLiteStorage& sqlite_storage,
        std::size_t const workers_count,
        std::size_t const available_batches_per_worker,
        std::size_t const batch_capacity,
        std::size_t const target_slice_bytes,
        std::atomic<std::size_t>& logs_operation_progress)
        : m_file_slice(file_slice)
        , m_shared_regex(shared_regex)
        , m_row_fields_count(row_fields_count)
        , m_sqlite_storage(sqlite_storage)
        , m_workers_count(workers_count)
        , m_available_batches_per_worker(available_batches_per_worker)
        , m_batch_capacity(batch_capacity)
        , m_target_slice_bytes(target_slice_bytes)
        , m_logs_operation_progress(logs_operation_progress)
    {
    }

    void Run()
    {
        LOG_SCOPE("::LogsImporter::Run()");

        auto file_slices = SplitFileSlice(m_file_slice, m_target_slice_bytes);
        auto const total_slices = file_slices.size();
        if (total_slices == 0)
        {
            return;
        }

        std::size_t const total_chunks =
            static_cast<std::size_t>(m_workers_count * m_available_batches_per_worker);

        DynamicChunkQueue queue(total_slices, total_chunks, m_batch_capacity, m_row_fields_count);

        auto writer_future = std::async(std::launch::async, [&]() {
            LOG_SCOPE("::LogsImporter::writer_thread");
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

                    if (!m_sqlite_storage.WriteChunk(chunk->rows, chunk->active_populated_rows))
                    {
                        LOG_ERROR("::LogsImporter::Run(): Failed to write chunk");
                    }

                    m_logs_operation_progress += chunk->chunk_size_bytes;

                    queue.RecycleChunk(std::move(chunk));
                    ++expected_chunk_idx;
                }
            }
        });

        std::vector<std::thread> workers{};
        {
            LOG_SCOPE("::ParserWorkerThreadsCreation()");
            workers.reserve(m_workers_count);
            std::atomic<std::size_t> next_slice_idx{0};
            auto const num_captures =
                static_cast<std::size_t>(m_shared_regex.NumberOfCapturingGroups());

            auto const importer_thread_id{std::this_thread::get_id()};
            for (unsigned int worker_idx = 0; worker_idx < m_workers_count; ++worker_idx)
            {
                workers.emplace_back([&, num_captures]() {
                    LOG_SCOPE(
                        "::ParserWorkerThread::{}::{}()",
                        importer_thread_id,
                        std::this_thread::get_id());

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

                        const char* ptr{file_slices[slice_idx].begin};
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

                            current_chunk->chunk_size_bytes +=
                                static_cast<std::size_t>(next_ptr - ptr);

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
                                auto& row =
                                    current_chunk->rows[current_chunk->active_populated_rows++];

                                for (std::size_t i = 0; i < num_captures && i < m_row_fields_count; ++i)
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

                                if (current_chunk->active_populated_rows == m_batch_capacity)
                                {
                                    queue.SubmitFilledChunk(std::move(current_chunk));
                                    current_chunk = queue.AcquireFreeChunk(slice_idx);
                                    current_chunk->chunk_index = local_chunk_idx++;
                                }
                            }

                            ptr = next_ptr;
                        }

                        if (current_chunk->active_populated_rows > 0 ||
                            current_chunk->chunk_size_bytes > 0)
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
    }

private:
    FileSlice m_file_slice;
    re2::RE2 const& m_shared_regex;
    std::size_t m_row_fields_count;
    SQLiteStorage& m_sqlite_storage;
    std::size_t m_workers_count;
    std::size_t m_available_batches_per_worker;
    std::size_t m_batch_capacity;
    std::size_t m_target_slice_bytes;
    std::atomic<std::size_t>& m_logs_operation_progress;
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
        LOG_SCOPE("::ImportLogs()::SliceWorkers()");

        auto const row_fields_count{m_imported_logs_header.size()};
        std::vector<std::thread> slice_workers{};
        slice_workers.reserve(mapped_file_slices.size());

        for (std::size_t slice_idx = 0; slice_idx < mapped_file_slices.size(); ++slice_idx)
        {
            slice_workers.emplace_back([slice_idx,
                                        slice = mapped_file_slices[slice_idx],
                                        &shared_regex,
                                        row_fields_count,
                                        this]() {
                LOG_SCOPE("::LogsImporter::Slice::{}()", slice_idx);
                Utility::Multithreading::LogsImporter importer(
                    slice,
                    shared_regex,
                    row_fields_count,
                    *m_sqlite_storages[slice_idx],
                    static_cast<std::size_t>(m_settings.import_params.workers_count),
                    static_cast<std::size_t>(m_settings.import_params.available_batches_per_worker),
                    static_cast<std::size_t>(m_settings.import_params.batch_capacity),
                    static_cast<std::size_t>(m_settings.import_params.file_target_slice_mb) * 1024 * 1024,
                    m_logs_operation_progress);

                importer.Run();
            });
        }

        for (auto& worker : slice_workers)
        {
            worker.join();
        }
    }

    {
        LOG_SCOPE("::ImportLogs()::CommitStorage()");
        std::vector<std::thread> commit_threads{};
        {
            LOG_SCOPE("::ImportLogs()::CommitStorage::ThreadsCreation()");
            commit_threads.reserve(m_sqlite_storages.size());
            for (auto const& storage : m_sqlite_storages)
            {
                commit_threads.emplace_back([&storage]() {
                    LOG_SCOPE("::ImportLogs()::CommitStorage::Thread()");
                    std::ignore = storage->Commit();
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
        std::size_t writtern_rows_total{0};
        for (auto const& storage : m_sqlite_storages)
        {
            writtern_rows_total += storage->GetWrittenRows();
        }
        m_filtered_logs.reserve(writtern_rows_total);
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

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V8
