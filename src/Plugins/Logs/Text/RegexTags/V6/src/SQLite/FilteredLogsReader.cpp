/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FilteredLogsReader.cpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Implementation of @see FilteredLogsReader.hpp
///

#include "FilteredLogsReader.hpp"

#include "Graphite/Common/Utility/UniqueID.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::FilteredLogsReader);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite::FilteredLogsReader);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite {

FilteredLogsReader::FilteredLogsReader(DatabaseRef db) : m_database{db}
{
    LOG_SCOPE("::FilteredLogsReader()");
}

Statement FilteredLogsReader::PrepareGetRangesQuery(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
    std::vector<std::string> const& fields)
{
    LOG_SCOPE("::PrepareGetRangesQuery()");

    if (ranges.empty())
    {
        LOG_WARN("::PrepareGetRangesQuery(): Ranges vector is empty.");
        return Statement{};
    }

    // Build fields list prefixed with table alias 'l.'
    std::string fields_sql_str{};
    for (std::size_t i = 0; i < fields.size(); ++i)
    {
        if (i > 0)
        {
            fields_sql_str += ", ";
        }
        fields_sql_str += "l." + fields[i];
    }

    // Join filtered_logs (f) and logs (l)
    std::string query_str = "SELECT " + fields_sql_str +
                            ", f.filter_id, f.highlight_filter_id, f.view_index " +
                            "FROM filtered_logs f " + "JOIN logs l ON f.log_id = l.id WHERE ";

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
        if (i > 0)
        {
            query_str += " OR ";
        }
        query_str += "(f.view_index >= " + std::to_string(ranges[i].begin) +
                     " AND f.view_index < " + std::to_string(ranges[i].end) + ")";
    }
    query_str += " ORDER BY f.view_index ASC;";

    LOG_INFO("::PrepareGetRangesQuery(): query == {}", query_str);

    Statement statement = m_database.Prepare(query_str);
    if (!statement.IsValid())
    {
        LOG_ERROR(
            "::PrepareGetRangesQuery(): Failed to prepare statement: {}",
            m_database.GetLastErrorMessage());
        return Statement{};
    }

    return statement;
}

bool FilteredLogsReader::NextFilteredRow(
    Statement& statement,
    std::vector<std::string>& out_fields,
    std::string& out_filter_id,
    std::string& out_highlight_id,
    std::size_t& out_view_index)
{
    LOG_SCOPE("::NextFilteredRow()");
    if (!statement.IsValid())
    {
        return false;
    }

    EStepResult const result = statement.Step();

    if (result == EStepResult::Row)
    {
        auto const col_count = static_cast<std::size_t>(statement.GetColumnCount());

        if (col_count < 3)
        {
            LOG_ERROR(
                "::NextFilteredRow(): Column count is unexpectedly less than 3 (total: {}).",
                col_count);
            return false;
        }

        std::size_t const num_fields = col_count - 3;
        out_fields.resize(num_fields);

        for (std::size_t idx = 0; idx < num_fields; ++idx)
        {
            out_fields[idx] = statement.GetColumnText(static_cast<int>(idx));
        }

        int const filter_col_idx = static_cast<int>(col_count - 3);
        int const highlight_col_idx = static_cast<int>(col_count - 2);
        int const view_index_col_idx = static_cast<int>(col_count - 1);

        auto const default_filter_id{Graphite::Common::Utility::UniqueID::Default().ToString()};

        // If filter_id is NULL in DB, fallback to default Graphite ID
        const char* filter_text = statement.GetColumnText(filter_col_idx);
        out_filter_id = filter_text ? filter_text : default_filter_id;

        // If highlight_filter_id is NULL in DB, fallback to default Graphite ID
        const char* highlight_text = statement.GetColumnText(highlight_col_idx);
        out_highlight_id = highlight_text ? highlight_text : default_filter_id;

        out_view_index = static_cast<std::size_t>(statement.GetColumnInt64(view_index_col_idx));

        return true;
    }
    else if (result == EStepResult::Done)
    {
        return false;
    }
    else
    {
        LOG_ERROR("::NextFilteredRow(): Execution error: {}", m_database.GetLastErrorMessage());
        return false;
    }
}

std::optional<std::size_t> FilteredLogsReader::GetNextFilteredIndex(
    std::string_view filter_id_str,
    std::size_t current_index)
{
    LOG_SCOPE("::GetNextFilteredIndex()");

    auto const default_filter_id{Graphite::Common::Utility::UniqueID::Default().ToString()};
    bool const is_default_filter = (filter_id_str == default_filter_id || filter_id_str.empty());

    auto execute_query = [this, &filter_id_str, is_default_filter](
                             std::string_view base_sql,
                             std::string_view wrap_sql,
                             std::size_t target_idx,
                             bool try_forward) -> std::optional<std::size_t> {
        std::string sql = try_forward ? std::string{base_sql} : std::string{wrap_sql};

        // If it's the default filter, adjust the WHERE clause to check for NULL
        if (is_default_filter)
        {
            if (try_forward)
            {
                sql =
                    "SELECT view_index FROM filtered_logs "
                    "WHERE (filter_id IS NULL OR highlight_filter_id IS NULL) AND view_index > ? "
                    "ORDER BY view_index ASC LIMIT 1;";
            }
            else
            {
                sql =
                    "SELECT view_index FROM filtered_logs "
                    "WHERE (filter_id IS NULL OR highlight_filter_id IS NULL) "
                    "ORDER BY view_index ASC LIMIT 1;";
            }
        }

        Statement stmt = m_database.Prepare(sql);
        if (!stmt.IsValid())
        {
            LOG_ERROR(
                "::GetNextFilteredIndex(): Failed to prepare statement: {}",
                m_database.GetLastErrorMessage());
            return std::nullopt;
        }

        if (is_default_filter)
        {
            if (try_forward)
            {
                stmt.BindInt64(1, static_cast<std::int64_t>(target_idx));
            }
        }
        else
        {
            stmt.BindText(1, std::string{filter_id_str});
            stmt.BindText(2, std::string{filter_id_str});
            if (try_forward)
            {
                stmt.BindInt64(3, static_cast<std::int64_t>(target_idx));
            }
        }

        if (stmt.Step() == EStepResult::Row)
        {
            return static_cast<std::size_t>(stmt.GetColumnInt64(0));
        }
        return std::nullopt;
    };

    static constexpr std::string_view forward_sql{
        "SELECT view_index FROM filtered_logs "
        "WHERE (filter_id = ? OR highlight_filter_id = ?) AND view_index > ? "
        "ORDER BY view_index ASC LIMIT 1;"};

    static constexpr std::string_view wrap_sql{
        "SELECT view_index FROM filtered_logs "
        "WHERE (filter_id = ? OR highlight_filter_id = ?) "
        "ORDER BY view_index ASC LIMIT 1;"};

    if (auto const found = execute_query(forward_sql, wrap_sql, current_index, true);
        found.has_value())
    {
        return found;
    }

    return execute_query(forward_sql, wrap_sql, current_index, false);
}

std::optional<std::size_t> FilteredLogsReader::GetPrevFilteredIndex(
    std::string_view filter_id_str,
    std::size_t current_index)
{
    LOG_SCOPE("::GetPrevFilteredIndex()");

    auto const default_filter_id{Graphite::Common::Utility::UniqueID::Default().ToString()};
    bool const is_default_filter = (filter_id_str == default_filter_id || filter_id_str.empty());

    auto execute_query = [this, &filter_id_str, is_default_filter](
                             std::string_view base_sql,
                             std::string_view wrap_sql,
                             std::size_t target_idx,
                             bool try_backward) -> std::optional<std::size_t> {
        std::string sql = try_backward ? std::string{base_sql} : std::string{wrap_sql};

        if (is_default_filter)
        {
            if (try_backward)
            {
                sql =
                    "SELECT view_index FROM filtered_logs "
                    "WHERE (filter_id IS NULL OR highlight_filter_id IS NULL) AND view_index < ? "
                    "ORDER BY view_index DESC LIMIT 1;";
            }
            else
            {
                sql =
                    "SELECT view_index FROM filtered_logs "
                    "WHERE (filter_id IS NULL OR highlight_filter_id IS NULL) "
                    "ORDER BY view_index DESC LIMIT 1;";
            }
        }

        Statement statement = m_database.Prepare(sql);
        if (!statement.IsValid())
        {
            LOG_ERROR(
                "::GetPrevFilteredIndex(): Failed to prepare statement: {}",
                m_database.GetLastErrorMessage());
            return std::nullopt;
        }

        if (is_default_filter)
        {
            if (try_backward)
            {
                statement.BindInt64(1, static_cast<std::int64_t>(target_idx));
            }
        }
        else
        {
            statement.BindText(1, std::string{filter_id_str});
            statement.BindText(2, std::string{filter_id_str});
            if (try_backward)
            {
                statement.BindInt64(3, static_cast<std::int64_t>(target_idx));
            }
        }

        if (statement.Step() == EStepResult::Row)
        {
            return static_cast<std::size_t>(statement.GetColumnInt64(0));
        }
        return std::nullopt;
    };

    static constexpr std::string_view backward_sql{
        "SELECT view_index FROM filtered_logs "
        "WHERE (filter_id = ? OR highlight_filter_id = ?) AND view_index < ? "
        "ORDER BY view_index DESC LIMIT 1;"};

    static constexpr std::string_view wrap_sql{
        "SELECT view_index FROM filtered_logs "
        "WHERE (filter_id = ? OR highlight_filter_id = ?) "
        "ORDER BY view_index DESC LIMIT 1;"};

    if (auto const found = execute_query(backward_sql, wrap_sql, current_index, true);
        found.has_value())
    {
        return found;
    }

    return execute_query(backward_sql, wrap_sql, current_index, false);
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite
