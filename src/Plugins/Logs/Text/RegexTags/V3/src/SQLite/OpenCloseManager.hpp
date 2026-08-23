
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file OpenCloseManager.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Wrapper for SQLite open/close operations
///

#pragma once

#include <filesystem>
#include <sqlite3.h>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

namespace Utility {

struct SQLiteDeleter
{
    void operator()(sqlite3* db) const;
};

} // namespace Utility

class OpenCloseManager
{
public: // Lifecycle
    OpenCloseManager(std::filesystem::path db_path);
    ~OpenCloseManager();

    OpenCloseManager(OpenCloseManager const&) = delete;
    OpenCloseManager& operator=(OpenCloseManager const&) = delete;

    OpenCloseManager(OpenCloseManager&&) = delete;
    OpenCloseManager& operator=(OpenCloseManager&&) = delete;

public: // public API
    /**
     * @brief Opens database
     *
     * @return true if opened successfuly, false otherwise
     */
    bool OpenDatabase();

protected: // Fields
    std::unique_ptr<sqlite3, Utility::SQLiteDeleter> m_db{nullptr};
    std::filesystem::path m_db_path{};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
