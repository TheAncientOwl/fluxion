/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SQLiteStorage.cpp
/// @author Alexandru Delegeanu
/// @version 9.0
/// @brief Implementation of @see SQLiteStorage.hpp
///

#include "SQLiteStorage.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

void SQLiteStorage::DatabaseDeleter::operator()(sqlite3* const database) const
{
    if (database)
    {
        sqlite3_close(database);
    }
}

void SQLiteStorage::StatementDeleter::operator()(sqlite3_stmt* const statement) const
{
    if (statement)
    {
        sqlite3_finalize(statement);
    }
}

SQLiteStorage::~SQLiteStorage()
{
    Close();
}

bool SQLiteStorage::Open(
    std::filesystem::path const& path,
    std::vector<std::string> const& fields,
    std::size_t const id_offset)
{
    Close();
    m_fields = fields;
    m_id_offset = id_offset;
    m_next_log_id = id_offset;

    sqlite3* database{nullptr};
    if (sqlite3_open(path.string().c_str(), &database) != SQLITE_OK)
    {
        DatabasePtr failed_database{database};
        Close();
        return false;
    }
    m_database.reset(database);

    std::string table_sql{"CREATE TABLE logs (id INTEGER PRIMARY KEY"};
    for (auto const& field : m_fields)
    {
        table_sql += ", \"" + field + "\" TEXT";
    }
    table_sql += ");";

    if (!Execute("PRAGMA journal_mode=OFF;") || !Execute("PRAGMA synchronous=OFF;") ||
        !Execute("PRAGMA locking_mode=EXCLUSIVE;") || !Execute(table_sql.c_str()))
    {
        Close();
        return false;
    }

    std::string insert_sql{"INSERT INTO logs (id"};
    for (auto const& field : m_fields)
    {
        insert_sql += ", \"" + field + "\"";
    }
    insert_sql += ") VALUES (?";
    for (std::size_t i = 0; i < m_fields.size(); ++i)
    {
        insert_sql += ", ?";
    }
    insert_sql += ");";

    sqlite3_stmt* insert_statement{nullptr};
    if (sqlite3_prepare_v2(m_database.get(), insert_sql.c_str(), -1, &insert_statement, nullptr) !=
        SQLITE_OK)
    {
        Close();
        return false;
    }
    m_insert_statement.reset(insert_statement);
    return true;
}

void SQLiteStorage::Close()
{
    m_insert_statement.reset();
    m_database.reset();
    m_next_log_id = 0;
    m_id_offset = 0;
}

bool SQLiteStorage::IsOpen() const
{
    return m_database != nullptr && m_insert_statement != nullptr;
}

std::size_t SQLiteStorage::GetIDOffset() const
{
    return m_id_offset;
}

std::size_t SQLiteStorage::GetWrittenRows() const
{
    return m_next_log_id - m_id_offset;
}

bool SQLiteStorage::Execute(char const* const sql) const
{
    return m_database != nullptr &&
           sqlite3_exec(m_database.get(), sql, nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool SQLiteStorage::BeginTransaction()
{
    return Execute("BEGIN TRANSACTION;");
}

bool SQLiteStorage::Commit()
{
    return Execute("COMMIT;");
}

bool SQLiteStorage::WriteChunk(
    std::vector<std::vector<std::string_view>> const& rows,
    std::size_t const active_rows)
{
    return WriteChunk(rows, active_rows, nullptr);
}

bool SQLiteStorage::WriteChunk(
    std::vector<std::vector<std::string_view>> const& rows,
    std::size_t const active_rows,
    std::vector<Data::FilteredLog>& out_filtered_logs)
{
    return WriteChunk(rows, active_rows, &out_filtered_logs);
}

bool SQLiteStorage::WriteChunk(
    std::vector<std::vector<std::string_view>> const& rows,
    std::size_t const active_rows,
    std::vector<Data::FilteredLog>* out_filtered_logs)
{
    if (!IsOpen())
    {
        return false;
    }
    std::lock_guard<std::mutex> lock{m_mutex};

    for (std::size_t row_index = 0; row_index < active_rows; ++row_index)
    {
        auto const& row = rows[row_index];
        auto const log_id = m_next_log_id;
        sqlite3_bind_int64(m_insert_statement.get(), 1, static_cast<sqlite3_int64>(log_id));
        for (std::size_t field_index = 0; field_index < m_fields.size(); ++field_index)
        {
            auto const value = field_index < row.size() ? row[field_index] : std::string_view{};
            sqlite3_bind_text(
                m_insert_statement.get(),
                static_cast<int>(field_index + 2),
                value.data(),
                static_cast<int>(value.size()),
                SQLITE_TRANSIENT);
        }

        if (sqlite3_step(m_insert_statement.get()) != SQLITE_DONE)
        {
            sqlite3_reset(m_insert_statement.get());
            sqlite3_clear_bindings(m_insert_statement.get());
            return false;
        }
        sqlite3_reset(m_insert_statement.get());
        sqlite3_clear_bindings(m_insert_statement.get());
        if (out_filtered_logs)
        {
            out_filtered_logs->emplace_back(log_id);
        }
        ++m_next_log_id;
    }
    return true;
}

bool SQLiteStorage::ReadAll(std::vector<std::pair<std::size_t, std::vector<std::string>>>& out_rows) const
{
    if (!IsOpen())
    {
        return false;
    }
    std::string sql{"SELECT id"};
    for (auto const& field : m_fields)
    {
        sql += ", \"" + field + "\"";
    }
    sql += " FROM logs ORDER BY id ASC;";

    sqlite3_stmt* statement{nullptr};
    if (sqlite3_prepare_v2(m_database.get(), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }
    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        std::vector<std::string> row;
        row.reserve(m_fields.size());
        for (std::size_t index = 0; index < m_fields.size(); ++index)
        {
            auto const* value = sqlite3_column_text(statement, static_cast<int>(index + 1));
            row.emplace_back(value ? reinterpret_cast<char const*>(value) : "");
        }
        out_rows.emplace_back(
            static_cast<std::size_t>(sqlite3_column_int64(statement, 0)), std::move(row));
    }
    sqlite3_finalize(statement);
    return true;
}

bool SQLiteStorage::ReadRows(RowCallback const callback) const
{
    if (!IsOpen() || !callback)
    {
        return false;
    }

    std::string sql{"SELECT id"};
    for (auto const& field : m_fields)
    {
        sql += ", \"" + field + "\"";
    }
    sql += " FROM logs ORDER BY id ASC;";

    sqlite3_stmt* statement{nullptr};
    if (sqlite3_prepare_v2(m_database.get(), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }

    bool completed{true};
    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        std::vector<std::string> row;
        row.reserve(m_fields.size());
        for (std::size_t index = 0; index < m_fields.size(); ++index)
        {
            auto const* value = sqlite3_column_text(statement, static_cast<int>(index + 1));
            row.emplace_back(value ? reinterpret_cast<char const*>(value) : "");
        }

        if (!callback(static_cast<std::size_t>(sqlite3_column_int64(statement, 0)), row))
        {
            completed = false;
            break;
        }
    }
    sqlite3_finalize(statement);
    return completed;
}

bool SQLiteStorage::ReadRowsByIDs(
    std::vector<Range> const& ranges,
    std::unordered_map<std::size_t, std::vector<std::string>>& out_rows) const
{
    if (!IsOpen() || ranges.empty())
    {
        return false;
    }
    std::string sql{"SELECT id"};
    for (auto const& field : m_fields)
    {
        sql += ", \"" + field + "\"";
    }
    sql += " FROM logs WHERE ";
    bool has_condition{false};
    for (auto const& range : ranges)
    {
        if (range.begin >= range.end)
        {
            continue;
        }
        if (has_condition)
        {
            sql += " OR ";
        }
        sql +=
            "(id >= " + std::to_string(range.begin) + " AND id < " + std::to_string(range.end) + ")";
        has_condition = true;
    }
    sql += ";";

    if (!has_condition)
    {
        return false;
    }

    sqlite3_stmt* statement{nullptr};
    if (sqlite3_prepare_v2(m_database.get(), sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
    {
        return false;
    }
    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        auto const id = static_cast<std::size_t>(sqlite3_column_int64(statement, 0));
        auto& row = out_rows[id];
        row.reserve(m_fields.size());
        for (std::size_t index = 0; index < m_fields.size(); ++index)
        {
            auto const* value = sqlite3_column_text(statement, static_cast<int>(index + 1));
            row.emplace_back(value ? reinterpret_cast<char const*>(value) : "");
        }
    }
    sqlite3_finalize(statement);
    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
