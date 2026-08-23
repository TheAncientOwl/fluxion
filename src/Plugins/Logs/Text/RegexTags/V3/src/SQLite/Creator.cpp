/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.cpp
/// @author Alexandru Delegeanu
/// @version 3.1
/// @brief Implementation of @see Creator.hpp
///

#include "Creator.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::Creator);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::Creator);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

Creator::Creator(DatabaseRef db) : m_db{db}
{
}

bool Creator::CreateTables(std::vector<std::string> const& fields_ids)
{
    LOG_SCOPE("::CreateTable()");

    // 1. Setup WAL
    if (!m_db.Execute("PRAGMA journal_mode=WAL;"))
    {
        LOG_ERROR("::CreateTable(): Failed to enable WAL mode: {}", m_db.GetLastErrorMessage());
        return false;
    }

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

    std::string err_msg{};
    if (!m_db.Execute(logs_table_sql, &err_msg))
    {
        LOG_ERROR(
            "::CreateTable(): Failed to create logs table: {}",
            err_msg.empty() ? "unknown error" : err_msg);
        return false;
    }

    // 3. Create filtered_logs table
    char const* filtered_logs_table_sql =
        "DROP TABLE IF EXISTS filtered_logs;"
        "CREATE TABLE filtered_logs ("
        "    view_index INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    log_id INTEGER NOT NULL,"
        "    filter_id TEXT,"
        "    highlight_filter_id TEXT,"
        "    FOREIGN KEY(log_id) REFERENCES logs(id)"
        ");";

    err_msg.clear();
    if (!m_db.Execute(filtered_logs_table_sql, &err_msg))
    {
        LOG_ERROR(
            "::CreateTable(): Failed to create filtered_logs table: {}",
            err_msg.empty() ? "unknown error" : err_msg);
        return false;
    }

    LOG_INFO("::CreateTable(): Tables created successfully.");

    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
