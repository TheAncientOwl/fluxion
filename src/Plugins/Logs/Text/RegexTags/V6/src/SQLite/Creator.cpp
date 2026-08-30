/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.cpp
/// @author Alexandru Delegeanu
/// @version 5.3
/// @brief Implementation of @see Creator.hpp
///

#include "Creator.hpp"
#include "Graphite/Logger.hpp"
#include "Wrapper/Transaction.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::Creator);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::Creator);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite {

Creator::Creator(DatabaseRef db) : m_database{db}
{
}

bool Creator::CreateTable(std::vector<std::string> const& fields_ids)
{
    LOG_SCOPE("::CreateTable()");

    // 1. Setup PRAGMAs
    if (!m_database.Execute("PRAGMA page_size=65536;") ||
        !m_database.Execute("PRAGMA journal_mode=OFF;") ||
        !m_database.Execute("PRAGMA synchronous=OFF;") ||
        !m_database.Execute("PRAGMA locking_mode=EXCLUSIVE;") ||
        !m_database.Execute("PRAGMA cache_size=-64000;") ||
        !m_database.Execute("PRAGMA temp_store=MEMORY;"))
    {
        LOG_ERROR(
            "::CreateTable(): Failed to set performance pragmas: {}",
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
        LOG_SCOPE("::CreateTable(): logs table");
        if (!m_database.Execute(logs_table_sql, &err_msg))
        {
            LOG_ERROR(
                "::CreateTable(): Failed to create logs table: {}",
                err_msg.empty() ? "unknown error" : err_msg);
            return false;
        }
    }

    if (!transaction.Commit())
    {
        LOG_ERROR(
            "::ExecuteFlush(): Failed to commit transaction: {}", m_database.GetLastErrorMessage());
        return false;
    }

    LOG_INFO("::CreateTable(): Tables created successfully.");

    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite
