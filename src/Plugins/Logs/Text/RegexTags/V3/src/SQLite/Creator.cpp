/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.cpp
/// @author Alexandru Delegeanu
/// @version 3.4
/// @brief Implementation of @see Creator.hpp
///

#include "Creator.hpp"
#include "Graphite/Logger.hpp"
#include "Wrapper/Transaction.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::Creator);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::Creator);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

Creator::Creator(DatabaseRef db) : m_database{db}
{
}

bool Creator::CreateTables(std::vector<std::string> const& fields_ids)
{
    LOG_SCOPE("::CreateTables()");

    // 1. Setup PRAGMAs
    if (!m_database.Execute("PRAGMA journal_mode=WAL;") ||
        !m_database.Execute("PRAGMA synchronous=NORMAL;") ||
        !m_database.Execute("PRAGMA cache_size=-64000;"))
    {
        LOG_ERROR(
            "::CreateTables(): Failed to set performance pragmas: {}",
            m_database.GetLastErrorMessage());
        return false;
    }

    Transaction transaction{m_database};
    if (!transaction.IsActive())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to begin transaction: {}", m_database.GetLastErrorMessage());
        return false;
    }

    // 2. Create logs table
    std::string logs_table_sql =
        "CREATE TABLE logs ("
        "id INTEGER PRIMARY KEY";

    for (auto const& field_id : fields_ids)
    {
        logs_table_sql += ", " + field_id + " TEXT";
    }
    logs_table_sql += ");";

    std::string err_msg{};
    {
        LOG_SCOPE("::CreateTables(): logs table");
        if (!m_database.Execute(logs_table_sql, &err_msg))
        {
            LOG_ERROR(
                "::CreateTables(): Failed to create logs table: {}",
                err_msg.empty() ? "unknown error" : err_msg);
            return false;
        }
    }

    // 3. Create filtered_logs table
    char const* filtered_logs_table_sql =
        "CREATE TABLE filtered_logs ("
        "    view_index INTEGER PRIMARY KEY,"
        "    log_id INTEGER NOT NULL,"
        "    filter_id TEXT,"
        "    highlight_filter_id TEXT,"
        "    FOREIGN KEY(log_id) REFERENCES logs(id)"
        ");";

    err_msg.clear();
    {
        LOG_SCOPE("::CreateTables(): filtered_logs table");
        if (!m_database.Execute(filtered_logs_table_sql, &err_msg))
        {
            LOG_ERROR(
                "::CreateTables(): Failed to create filtered_logs table: {}",
                err_msg.empty() ? "unknown error" : err_msg);
            return false;
        }
    }

    LOG_INFO("::CreateTables(): Tables created successfully.");

    if (!transaction.Commit())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to commit transaction: {}", m_database.GetLastErrorMessage());
        return false;
    }

    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
