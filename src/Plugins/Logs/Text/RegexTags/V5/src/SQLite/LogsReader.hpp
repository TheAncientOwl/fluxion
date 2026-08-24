/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsReader.hpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Wrapper for reading raw logs from SQLite
///

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Wrapper/DatabaseRef.hpp"
#include "Wrapper/Statement.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite {

class LogsReader
{
public: // Lifecycle
    explicit LogsReader(DatabaseRef db);
    ~LogsReader() = default;

    LogsReader(LogsReader const&) = delete;
    LogsReader& operator=(LogsReader const&) = delete;

    LogsReader(LogsReader&&) noexcept = default;
    LogsReader& operator=(LogsReader&&) noexcept = default;

public: // Public API
    /**
     * @brief Prepares a SQL query to fetch all raw logs and their IDs from the logs table.
     * @param fields Vector of column field names to select.
     * @return Statement managing the statement lifetime.
     */
    Statement PrepareGetAllLogsQuery(std::vector<std::string> const& fields);

    /**
     * @brief Fetches the next raw log row, populating the log ID and field values.
     * @param statement The active Statement wrapper.
     * @param out_log_id Output primary key (id) of the log.
     * @param out_fields Vector to populate with the dynamic log field values.
     * @return true if a row was read successfully, false if no more rows exist.
     */
    bool NextRow(Statement& statement, std::size_t& out_log_id, std::vector<std::string>& out_fields);

private:
    DatabaseRef m_database;
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite
