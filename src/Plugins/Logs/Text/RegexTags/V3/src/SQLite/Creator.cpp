
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation of @see Creator.hpp
///

#include "Creator.hpp"

#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::Creator);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::Creator);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

Creator::Creator(std::filesystem::path db_path) : OpenCloseManager{std::move(db_path)}
{
}

bool Creator::CreateTable(std::vector<std::string> const& fields_ids)
{
    LOG_SCOPE("::CreateTable()");
    if (!m_db)
    {
        LOG_ERROR("::CreateTable(): Database is not open!");
        return false;
    }

    // 1. Setup WAL
    sqlite3_exec(m_db.get(), "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    // 2. Create logs table
    std::string logs_table_sql =
        "DROP TABLE IF EXISTS logs;"
        "CREATE TABLE logs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT";

    for (auto const& field_id : fields_ids)
    {
        logs_table_sql += ", " + field_id + " TEXT";
    }
    logs_table_sql += ");";

    char* err_msg = nullptr;
    auto return_code = sqlite3_exec(m_db.get(), logs_table_sql.c_str(), nullptr, nullptr, &err_msg);

    if (return_code != SQLITE_OK)
    {
        LOG_ERROR(
            "::CreateTable(): Failed to create logs table: {}", err_msg ? err_msg : "unknown error");
        if (err_msg)
        {
            sqlite3_free(err_msg);
        }
        return false;
    }

    // 3. Create filtered_logs table
    const char* filtered_logs_table_sql =
        "DROP TABLE IF EXISTS filtered_logs;"
        "CREATE TABLE filtered_logs ("
        "    view_index INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    log_id INTEGER NOT NULL,"
        "    filter_id TEXT,"
        "    highlight_filter_id TEXT,"
        "    FOREIGN KEY(log_id) REFERENCES logs(id)"
        ");";
    err_msg = nullptr;
    return_code = sqlite3_exec(m_db.get(), filtered_logs_table_sql, nullptr, nullptr, &err_msg);

    if (return_code != SQLITE_OK)
    {
        LOG_ERROR(
            "::CreateTable(): Failed to create filtered_logs table: {}",
            err_msg ? err_msg : "unknown error");
        if (err_msg)
        {
            sqlite3_free(err_msg);
        }
        return false;
    }

    LOG_INFO("::CreateTable(): Tables created successfully.");

    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
