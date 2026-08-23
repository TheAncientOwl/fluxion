/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file OpenCloseManager.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation of @see OpenCloseManager.hpp
///

#include "OpenCloseManager.hpp"

#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::OpenCloseManager);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::OpenCloseManager);

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

OpenCloseManager::OpenCloseManager(std::filesystem::path db_path)
    : m_db{nullptr}, m_db_path{std::move(db_path)}
{
}

OpenCloseManager::~OpenCloseManager()
{
}

bool OpenCloseManager::OpenDatabase()
{
    LOG_SCOPE("::OpenDatabase()");
    sqlite3* raw_db = nullptr;

#if defined(_WIN32)
    auto const db_path = m_db_path.u8string();
#else
    auto const db_path = m_db_path.string();
#endif

    auto const return_code = sqlite3_open_v2(
        db_path.c_str(), &raw_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (return_code != SQLITE_OK)
    {
        char const* err_msg = raw_db ? sqlite3_errmsg(raw_db) : "unknown error";
        LOG_ERROR(
            "::OpenDatabase(): Failed to open db at \"{}\", error code {}, message: {}",
            db_path,
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

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
