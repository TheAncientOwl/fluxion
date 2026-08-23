/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FilteredLogsReader.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Wrapper for SQLite read operations over logs & filtered_logs tables
///

#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>

#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"
#include "OpenCloseManager.hpp"
#include "QueryHandle.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

class FilteredLogsReader : public OpenCloseManager
{
public: // Lifecycle
    FilteredLogsReader(std::filesystem::path db_path);
    ~FilteredLogsReader() = default;

public: // Public API
    /**
     * @brief Prepares a SQL query targeting specific row ranges using filtered_view_index.
     * @param ranges Vector of ranges [begin, end) to fetch.
     * @return QueryHandle managing the statement lifetime.
     */
    QueryHandle PrepareGetRangesQuery(
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
        std::vector<std::string> const& fields);

    /**
     * @brief Fetches the next row, populating log fields, metadata, and the view index.
     * @param query The active QueryHandle.
     * @param out_fields Vector to populate with the dynamic log fields (0..N-1).
     * @param out_filter_id Output string for filter_id.
     * @param out_highlight_id Output string for highlight_filter_id.
     * @param out_view_index Output value for filtered_view_index.
     * @return true if a row was read successfully, false if no more rows exist.
     */
    bool NextFilteredRow(
        QueryHandle& query,
        std::vector<std::string>& out_fields,
        std::string& out_filter_id,
        std::string& out_highlight_id,
        std::size_t& out_view_index);

    void ChangeDatabase(std::filesystem::path new_db_path);

    /**
     * @brief Finds the next view index matching the given filter ID, with wrap-around.
     * @param filter_id_str String representation of the UniqueID to search for.
     * @param current_index The current 0-based view index.
     * @return std::optional containing the next matching view index if found.
     */
    [[nodiscard]] std::optional<std::size_t> GetNextFilteredIndex(
        std::string_view filter_id_str,
        std::size_t current_index);

    /**
     * @brief Finds the previous view index matching the given filter ID, with wrap-around.
     * @param filter_id_str String representation of the UniqueID to search for.
     * @param current_index The current 0-based view index.
     * @return std::optional containing the previous matching view index if found.
     */
    [[nodiscard]] std::optional<std::size_t> GetPrevFilteredIndex(
        std::string_view filter_id_str,
        std::size_t current_index);
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
