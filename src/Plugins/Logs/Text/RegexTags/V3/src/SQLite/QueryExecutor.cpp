/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file QueryExecutor.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation of @see QueryExecutor.hpp.
///

#include "QueryExecutor.hpp"
#include "Graphite/Logger.hpp"
#include "sqlite3.h"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::QueryExecutor);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::QueryExecutor);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

QueryExecutor::QueryExecutor(std::filesystem::path const& db_path) noexcept
    : OpenCloseManager{db_path}
{
    LOG_SCOPE("::QueryExecutor()");
}

bool QueryExecutor::Execute(std::string_view const sql_query) const
{
    LOG_SCOPE("::Execute()");

    char* err_msg{nullptr};
    int const rc{sqlite3_exec(m_db.get(), sql_query.data(), nullptr, nullptr, &err_msg)};
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(
            "::Execute(): SQLite execution failed: {} (Query: {})",
            err_msg ? err_msg : "unknown error",
            sql_query);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool QueryExecutor::ExecuteTransaction(std::function<bool()> const& transaction_func) const
{
    LOG_SCOPE("::ExecuteTransaction()");
    if (!Execute("BEGIN TRANSACTION;"))
    {
        LOG_ERROR("::ExecuteTransaction(): Failed to begin transaction.");
        return false;
    }

    if (!transaction_func())
    {
        LOG_WARN("::ExecuteTransaction(): Transaction operation failed, rolling back.");
        std::ignore = Execute("ROLLBACK;");
        return false;
    }

    if (!Execute("COMMIT;"))
    {
        LOG_ERROR("::ExecuteTransaction(): Failed to commit transaction.");
        return false;
    }

    return true;
}

void QueryExecutor::AddProgressHandler(int const opcode_intervals, ProgressCallback callback)
{
    LOG_SCOPE("::AddProgressHandler()");

    m_progress_callback = std::move(callback);
    sqlite3_progress_handler(
        m_db.get(), opcode_intervals, &QueryExecutor::SQLiteProgressCallbackTrampoline, this);
}

void QueryExecutor::RemoveProgressHandler()
{
    LOG_SCOPE("::RemoveProgressHandler()");

    m_progress_callback = nullptr;
    sqlite3_progress_handler(m_db.get(), 0, nullptr, nullptr);
}

int QueryExecutor::SQLiteProgressCallbackTrampoline(void* arg)
{
    auto* executor{static_cast<QueryExecutor*>(arg)};
    if (executor && executor->m_progress_callback)
    {
        return executor->m_progress_callback();
    }
    return 0;
}

[[nodiscard]] std::size_t QueryExecutor::GetFilteredLogsCount() const
{
    sqlite3_stmt* stmt{nullptr};
    char const* sql{"SELECT COUNT(*) FROM filtered_logs;"};
    if (sqlite3_prepare_v2(m_db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return 0;
    }

    std::size_t count{0};
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = static_cast<std::size_t>(sqlite3_column_int64(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return count;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
