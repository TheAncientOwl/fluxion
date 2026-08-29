/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ConnectionManager.cpp
/// @author Alexandru Delegeanu
/// @version 3.1
/// @brief Implementation of @see ConnectionManager.hpp
///

#include "ConnectionManager.hpp"

#include "Graphite/Logger.hpp"

#include <sqlite3.h>

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::ConnectionManager);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::ConnectionManager);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

namespace Utility {

void SQLiteDeleter::operator()(sqlite3* db) const
{
    if (db)
    {
        sqlite3_close(db);
    }
}

} // namespace Utility

ConnectionManager::ConnectionManager() : m_db{nullptr}
{
}

ConnectionManager::~ConnectionManager() = default;

bool ConnectionManager::OpenDatabase(std::filesystem::path const& path)
{
    LOG_SCOPE("::OpenDatabase()");

    m_db.reset();

    sqlite3* raw_db = nullptr;
    auto return_code = SQLITE_OK;

#if defined(_WIN32)
    return_code = sqlite3_open16(path.c_str(), &raw_db);
#else
    auto const db_path = path.string();
    return_code = sqlite3_open_v2(
        db_path.c_str(), &raw_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
#endif

    if (return_code != SQLITE_OK)
    {
        char const* err_msg = raw_db ? sqlite3_errmsg(raw_db) : "unknown error";
        LOG_ERROR(
            "::OpenDatabase(): Failed to open db at \"{}\", error code {}, message: {}",
            path,
            return_code,
            err_msg);

        if (raw_db)
        {
            sqlite3_close(raw_db);
        }
        return false;
    }

    m_db.reset(raw_db);
    return true;
}

DatabaseRef ConnectionManager::GetDatabaseRef() const
{
    return DatabaseRef{m_db.get()};
}

bool ConnectionManager::IsOpen() const
{
    return m_db != nullptr;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
