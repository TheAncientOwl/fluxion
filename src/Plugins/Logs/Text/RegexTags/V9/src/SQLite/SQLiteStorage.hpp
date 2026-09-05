/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SQLiteStorage.hpp
/// @author Alexandru Delegeanu
/// @version 9.5
/// @brief SQLite operations manager
///

#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <sqlite3.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/Data.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

class SQLiteStorage
{
public:
    using RowViewCallback = std::function<bool(std::size_t, std::vector<std::string_view> const&)>;

    struct Range
    {
        std::size_t begin{0};
        std::size_t end{0};
    };

    ///
    /// @brief Constructs an empty SQLite storage object.
    ///
    /// @return Nothing.
    ///
    SQLiteStorage() = default;

    ///
    /// @brief Closes the database and releases SQLite resources.
    ///
    /// @return Nothing.
    ///
    ~SQLiteStorage();

    ///
    /// @brief Prevents copying SQLite storage state.
    ///
    /// @param other Storage object to copy.
    /// @return Not available; copying is deleted.
    ///
    SQLiteStorage(SQLiteStorage const&) = delete;

    ///
    /// @brief Prevents copy assignment of SQLite storage state.
    ///
    /// @param other Storage object to copy.
    /// @return Not available; copy assignment is deleted.
    ///
    SQLiteStorage& operator=(SQLiteStorage const&) = delete;

    ///
    /// @brief Prevents moving SQLite storage state.
    ///
    /// @param other Storage object to move.
    /// @return Not available; moving is deleted.
    ///
    SQLiteStorage(SQLiteStorage&&) noexcept = delete;

    ///
    /// @brief Prevents move assignment of SQLite storage state.
    ///
    /// @param other Storage object to move.
    /// @return Not available; move assignment is deleted.
    ///
    SQLiteStorage& operator=(SQLiteStorage&&) noexcept = delete;

    ///
    /// @brief Opens a SQLite database and creates its logs table.
    ///
    /// @param path Database file path.
    /// @param fields Log column names.
    /// @param id_offset First log ID assigned by this storage.
    /// @return True when the database and insert statement are ready.
    ///
    bool Open(
        std::filesystem::path const& path,
        std::vector<std::string> const& fields,
        std::size_t const id_offset = 0);

    ///
    /// @brief Closes the database and releases SQLite resources.
    ///
    /// @return Nothing.
    ///
    void Close();

    ///
    /// @brief Checks whether the storage is open and ready.
    ///
    /// @return True when the database and insert statement are valid.
    ///
    bool IsOpen() const;

    ///
    /// @brief Gets the first ID assigned by this storage.
    ///
    /// @return The configured ID offset.
    ///
    std::size_t GetIDOffset() const;

    ///
    /// @brief Gets the number of successfully written rows.
    ///
    /// @return The number of rows written since opening the storage.
    ///
    std::size_t GetWrittenRows() const;

    ///
    /// @brief Begins a SQLite transaction.
    ///
    /// @return True when the transaction starts successfully.
    ///
    bool BeginTransaction();

    ///
    /// @brief Commits the current SQLite transaction.
    ///
    /// @return True when the transaction commits successfully.
    ///
    bool Commit();

    ///
    /// @brief Writes the active rows in a chunk.
    ///
    /// @param rows Chunk rows to write.
    /// @param active_rows Number of valid rows in the chunk.
    /// @return True when every row is written successfully.
    ///
    bool WriteChunk(std::vector<std::vector<std::string_view>> const& rows, std::size_t const active_rows);

    ///
    /// @brief Writes a chunk and records the generated log IDs.
    ///
    /// @param rows Chunk rows to write.
    /// @param active_rows Number of valid rows in the chunk.
    /// @param out_filtered_logs Receives the generated log IDs.
    /// @return True when every row is written successfully.
    ///
    bool WriteChunk(
        std::vector<std::vector<std::string_view>> const& rows,
        std::size_t const active_rows,
        std::vector<Data::FilteredLog>& out_filtered_logs);

    ///
    /// @brief Writes a chunk without locking for a dedicated single writer.
    ///
    /// @warning The caller must guarantee exclusive access to this storage.
    ///
    /// @param rows Chunk rows to write.
    /// @param active_rows Number of valid rows in the chunk.
    /// @return True when every row is written successfully.
    ///
    bool WriteChunkSingleWriter(
        std::vector<std::vector<std::string_view>> const& rows,
        std::size_t const active_rows);

    ///
    /// @brief Streams rows to a callback without materializing field strings.
    ///
    /// @param callback Called for each row; views are valid only during the callback.
    /// @return True when all rows are visited, false when stopped or on error.
    ///
    bool ReadRowsViews(RowViewCallback const callback) const;

    ///
    /// @brief Reads rows whose IDs fall within any supplied half-open range into existing vectors.
    ///
    /// @param ranges ID ranges in the form [begin, end).
    /// @param out_rows Maps log IDs to destination row vectors.
    /// @return True when the query completes successfully.
    ///
    bool ReadRowsByIDsInto(
        std::vector<Range> const& ranges,
        std::unordered_map<std::size_t, std::vector<std::string>*> const& out_rows) const;

private:
    struct DatabaseDeleter
    {
        void operator()(sqlite3* database) const;
    };

    struct StatementDeleter
    {
        void operator()(sqlite3_stmt* statement) const;
    };

    using DatabasePtr = std::unique_ptr<sqlite3, DatabaseDeleter>;
    using StatementPtr = std::unique_ptr<sqlite3_stmt, StatementDeleter>;

    ///
    /// @brief Executes a SQL statement without result rows.
    ///
    /// @param sql SQL statement to execute.
    /// @return True when SQLite executes the statement successfully.
    ///
    bool Execute(char const* const sql) const;

    ///
    /// @brief Writes a chunk and optionally records generated IDs.
    ///
    /// @param rows Chunk rows to write.
    /// @param active_rows Number of valid rows in the chunk.
    /// @param out_filtered_logs Optional output for generated log IDs.
    /// @return True when every row is written successfully.
    ///
    bool WriteChunk(
        std::vector<std::vector<std::string_view>> const& rows,
        std::size_t const active_rows,
        std::vector<Data::FilteredLog>* out_filtered_logs);

    bool WriteChunkUnlocked(
        std::vector<std::vector<std::string_view>> const& rows,
        std::size_t const active_rows,
        std::vector<Data::FilteredLog>* out_filtered_logs);

private:
    DatabasePtr m_database{nullptr};
    StatementPtr m_insert_statement{nullptr};
    std::vector<std::string> m_fields{};
    std::string m_select_columns_sql{};
    std::size_t m_id_offset{0};
    std::size_t m_next_log_id{0};
    mutable std::mutex m_mutex{};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
