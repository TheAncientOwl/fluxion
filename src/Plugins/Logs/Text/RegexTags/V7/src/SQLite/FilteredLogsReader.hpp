/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FilteredLogsReader.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Wrapper for SQLite read operations over logs table and in-memory filtered logs navigation.
///

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/Data.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"
#include "Wrapper/DatabaseRef.hpp"
#include "Wrapper/Statement.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite {

class FilteredLogsReader
{
public: // Lifecycle
    explicit FilteredLogsReader(DatabaseRef db);
    ~FilteredLogsReader() = default;

    FilteredLogsReader(FilteredLogsReader const&) = delete;
    FilteredLogsReader& operator=(FilteredLogsReader const&) = delete;

    FilteredLogsReader(FilteredLogsReader&&) noexcept = default;
    FilteredLogsReader& operator=(FilteredLogsReader&&) noexcept = default;

public: // Public API
    /**
     * @brief Prepares a SQL query targeting raw log fields by log ID.
     * @param log_ids Vector of log IDs (primary keys in the logs table) to fetch.
     * @param fields Column field names to select from the logs table.
     * @return Statement managing the prepared statement lifetime.
     */
    Statement PrepareGetLogsByIDsQuery(
        std::vector<std::uint64_t> const& log_ids,
        std::vector<std::string> const& fields);

    /**
     * @brief Finds the next view index matching the given filter ID, with wrap-around.
     * @param filtered_logs In-memory vector containing filtered log entries.
     * @param target_filter_id UniqueID filter to search for across filter and highlight fields.
     * @param current_index The current 0-based view index.
     * @return std::optional containing the next matching view index if found.
     */
    [[nodiscard]] std::optional<std::size_t> GetNextFilteredIndex(
        std::vector<Data::FilteredLog> const& filtered_logs,
        Graphite::Common::Utility::UniqueID const& target_filter_id,
        std::size_t current_index);

    /**
     * @brief Finds the previous view index matching the given filter ID, with wrap-around.
     * @param filtered_logs In-memory vector containing filtered log entries.
     * @param target_filter_id UniqueID filter to search for across filter and highlight fields.
     * @param current_index The current 0-based view index.
     * @return std::optional containing the previous matching view index if found.
     */
    [[nodiscard]] std::optional<std::size_t> GetPrevFilteredIndex(
        std::vector<Data::FilteredLog> const& filtered_logs,
        Graphite::Common::Utility::UniqueID const& target_filter_id,
        std::size_t current_index);

private:
    DatabaseRef m_database;
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite
