/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FilteredLogsReader.hpp
/// @author Alexandru Delegeanu
/// @version 4.0
/// @brief Wrapper for SQLite read operations over logs & filtered_logs tables
///

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"
#include "Wrapper/DatabaseRef.hpp"
#include "Wrapper/Statement.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V4::SQLite {

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
     * @brief Prepares a SQL query targeting specific row ranges using filtered_view_index.
     * @param ranges Vector of ranges [begin, end) to fetch.
     * @return Statement managing the prepared statement lifetime.
     */
    Statement PrepareGetRangesQuery(
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
        std::vector<std::string> const& fields);

    /**
     * @brief Fetches the next row, populating log fields, metadata, and the view index.
     * @param statement The active Statement wrapper.
     * @param out_fields Vector to populate with the dynamic log fields (0..N-1).
     * @param out_filter_id Output string for filter_id.
     * @param out_highlight_id Output string for highlight_filter_id.
     * @param out_view_index Output value for filtered_view_index.
     * @return true if a row was read successfully, false if no more rows exist.
     */
    bool NextFilteredRow(
        Statement& statement,
        std::vector<std::string>& out_fields,
        std::string& out_filter_id,
        std::string& out_highlight_id,
        std::size_t& out_view_index);

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

private:
    DatabaseRef m_database;
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V4::SQLite
