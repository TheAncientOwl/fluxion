/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsReader.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation of @see LogsReader.hpp
///

#include "LogsReader.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::LogsReader);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite::LogsReader);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

LogsReader::LogsReader(std::filesystem::path db_path) : OpenCloseManager{std::move(db_path)}
{
    LOG_SCOPE("::LogsReader()");
}

QueryHandle LogsReader::PrepareGetAllLogsQuery(std::vector<std::string> const& fields)
{
    LOG_SCOPE("::PrepareGetAllLogsQuery()");
    if (!m_db)
    {
        LOG_ERROR("::PrepareGetAllLogsQuery(): Database is not open!");
        return QueryHandle{};
    }

    if (fields.empty())
    {
        LOG_WARN("::PrepareGetAllLogsQuery(): Fields vector is empty.");
        return QueryHandle{};
    }

    std::string fields_sql_str{"id"};
    for (auto const& field : fields)
    {
        fields_sql_str += ", " + field;
    }

    std::string const query_str = "SELECT " + fields_sql_str + " FROM logs ORDER BY id ASC;";
    LOG_INFO("::PrepareGetAllLogsQuery(): query == {}", query_str);

    sqlite3_stmt* stmt = nullptr;
    auto const rc = sqlite3_prepare_v2(m_db.get(), query_str.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK)
    {
        LOG_ERROR(
            "::PrepareGetAllLogsQuery(): Failed to prepare statement: {}", sqlite3_errmsg(m_db.get()));
        return QueryHandle{};
    }

    return QueryHandle{stmt};
}

bool LogsReader::NextRow(QueryHandle& query, std::size_t& out_log_id, std::vector<std::string>& out_fields)
{
    if (!query)
    {
        return false;
    }

    int const rc = sqlite3_step(query.Get());

    if (rc == SQLITE_ROW)
    {
        auto const col_count = static_cast<std::size_t>(sqlite3_column_count(query.Get()));
        if (col_count < 2)
        {
            LOG_ERROR("::NextRow(): Column count is unexpectedly less than 2 (total: {}).", col_count);
            return false;
        }

        // Column 0 is logs.id
        out_log_id = static_cast<std::size_t>(sqlite3_column_int64(query.Get(), 0));

        // Remaining columns are fields (1 .. col_count - 1)
        std::size_t const num_fields = col_count - 1;
        out_fields.resize(num_fields);

        for (std::size_t idx = 0; idx < num_fields; ++idx)
        {
            char const* text = reinterpret_cast<char const*>(
                sqlite3_column_text(query.Get(), static_cast<int>(idx + 1)));
            out_fields[idx] = text ? text : "";
        }

        return true;
    }
    else if (rc == SQLITE_DONE)
    {
        return false;
    }
    else
    {
        LOG_ERROR("::NextRow(): Execution error: {}", sqlite3_errmsg(m_db.get()));
        return false;
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
