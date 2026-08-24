/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DatabaseRef.cpp
/// @author Alexandru Delegeanu
/// @version 3.1
/// @brief Implementation of @see DatabaseRef.hpp
///

#include "DatabaseRef.hpp"

#include <sqlite3.h>
#include <tuple>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

DatabaseRef::DatabaseRef(sqlite3* db) : m_db{db}
{
}

bool DatabaseRef::Execute(std::string_view sql, std::string* out_err) const
{
    char* err_msg = nullptr;
    int const rc = sqlite3_exec(m_db, sql.data(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK)
    {
        if (out_err && err_msg)
        {
            *out_err = err_msg;
        }
        if (err_msg)
        {
            sqlite3_free(err_msg);
        }
        return false;
    }

    return true;
}

bool DatabaseRef::ExecuteTransaction(std::function<bool()> const& transaction_func) const
{
    if (!Execute("BEGIN TRANSACTION;"))
    {
        return false;
    }

    if (!transaction_func())
    {
        std::ignore = Execute("ROLLBACK;");
        return false;
    }

    if (!Execute("COMMIT;"))
    {
        return false;
    }

    return true;
}

Statement DatabaseRef::Prepare(std::string_view sql) const
{
    sqlite3_stmt* raw_stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql.data(), -1, &raw_stmt, nullptr) != SQLITE_OK)
    {
        return Statement{nullptr};
    }
    return Statement{raw_stmt};
}

std::size_t DatabaseRef::GetFilteredLogsCount() const
{
    Statement stmt = Prepare("SELECT COUNT(*) FROM filtered_logs;");
    if (!stmt.IsValid())
    {
        return 0;
    }

    if (stmt.Step() == EStepResult::Row)
    {
        return static_cast<std::size_t>(stmt.GetColumnInt64(0));
    }

    return 0;
}

char const* DatabaseRef::GetLastErrorMessage() const
{
    return sqlite3_errmsg(m_db);
}

void DatabaseRef::SetProgressHandler(int const num_opcodes, ProgressCallback const& callback) const
{
    auto trampoline = [](void* user_data) -> int {
        auto* cb = static_cast<ProgressCallback const*>(user_data);
        return (cb && *cb) ? (*cb)() : 0;
    };

    sqlite3_progress_handler(
        m_db, num_opcodes, trampoline, const_cast<void*>(static_cast<void const*>(&callback)));
}

void DatabaseRef::ClearProgressHandler() const
{
    sqlite3_progress_handler(m_db, 0, nullptr, nullptr);
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
