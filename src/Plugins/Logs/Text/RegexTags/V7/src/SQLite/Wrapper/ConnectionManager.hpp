/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ConnectionManager.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Wrapper for SQLite open/close operations
///

#pragma once

#include "DatabaseRef.hpp"

#include <filesystem>
#include <memory>

struct sqlite3;

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite {

namespace Utility {

struct SQLiteDeleter
{
    void operator()(sqlite3* db) const;
};

} // namespace Utility

class ConnectionManager
{
public: // Lifecycle
    ConnectionManager();
    ~ConnectionManager();

    ConnectionManager(ConnectionManager const&) = delete;
    ConnectionManager& operator=(ConnectionManager const&) = delete;

    ConnectionManager(ConnectionManager&&) = delete;
    ConnectionManager& operator=(ConnectionManager&&) = delete;

public: // Public API
    /**
     * @brief Opens database
     *
     * @param path Path to the SQLite database file
     * @return true if opened successfully, false otherwise
     */
    [[nodiscard]] bool OpenDatabase(std::filesystem::path const& path);
    void Close();

    [[nodiscard]] bool IsOpen() const;

    [[nodiscard]] DatabaseRef GetDatabaseRef() const;

protected: // Fields
    std::unique_ptr<sqlite3, Utility::SQLiteDeleter> m_db{nullptr};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite
