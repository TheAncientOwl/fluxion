/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsReader.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Wrapper for reading raw logs from SQLite
///

#pragma once

#include <filesystem>
#include <sqlite3.h>
#include <string>
#include <vector>
#include "OpenCloseManager.hpp"
#include "QueryHandle.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

class LogsReader : public OpenCloseManager
{
public: // Lifecycle
    explicit LogsReader(std::filesystem::path db_path);
    ~LogsReader() = default;

public: // Public API
    /**
     * @brief Prepares a SQL query to fetch all raw logs and their IDs from the logs table.
     * @param fields Vector of column field names to select.
     * @return QueryHandle managing the statement lifetime.
     */
    QueryHandle PrepareGetAllLogsQuery(std::vector<std::string> const& fields);

    /**
     * @brief Fetches the next raw log row, populating the log ID and field values.
     * @param query The active QueryHandle.
     * @param out_log_id Output primary key (id) of the log.
     * @param out_fields Vector to populate with the dynamic log field values.
     * @return true if a row was read successfully, false if no more rows exist.
     */
    bool NextRow(QueryHandle& query, std::size_t& out_log_id, std::vector<std::string>& out_fields);
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
