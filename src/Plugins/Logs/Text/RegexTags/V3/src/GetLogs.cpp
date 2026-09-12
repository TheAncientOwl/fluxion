/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 3.2.0
/// @brief Implementation @see RegexTags.hpp
///

#include <filesystem>
#include <span>
#include <system_error>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V3/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"
#include "SQLite/Utility.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::GetLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::GetLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3 {

void RegexTags::GetLogsABI(Bridge::ABI::Span<Bridge::Range> const _ranges, Bridge::ILogsWriter* out_logs)
{
    LOG_SCOPE("::GetLogsABI()");

    if (!out_logs)
    {
        return;
    }

    std::span<Bridge::Range const> const ranges{_ranges};

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::GetLogsABI(): SQLite connection is closed and could not be opened");
        return;
    }

    std::stringstream ss{};
    for (auto range : ranges)
    {
        ss << "[" << range.begin << ", " << range.end << "), ";
    }
    LOG_INFO("::GetLogsABI(): Requested ranges: {}", ss.str());

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogsABI(): No logs were imported before");
        return;
    }

    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    if (!static_cast<bool>(total_logs_opt))
    {
        LOG_WARN("::GetLogsABI(): total_logs is not set in config");
        return;
    }

    std::error_code ec;
    auto const db_path = MakeDatabasePath(*m_last_imported_logs_path);
    if (std::filesystem::file_size(db_path, ec) == 0 || ec)
    {
        LOG_WARN(
            "::GetLogsABI(): Database file {} is currently 0 bytes or locked. Skipping read.", db_path);
        return;
    }

    if (m_imported_logs_header.empty())
    {
        LOG_WARN("::GetLogsABI(): m_imported_logs_header is empty.");
        return;
    }

    auto reader{SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}};
    auto query_handle{reader.PrepareGetRangesQuery(
        ranges, SQLite::Utility::MakeFieldsIDs(m_imported_logs_header))};
    if (!query_handle.IsValid())
    {
        LOG_ERROR("::GetLogsABI(): Failed to prepare ranges query.");
        return;
    }

    std::vector<std::string> row_fields;
    std::string filter_id_str;
    std::string highlight_id_str;
    std::size_t view_index = 0;

    while (reader.NextFilteredRow(query_handle, row_fields, filter_id_str, highlight_id_str, view_index))
    {
        if (view_index >= *total_logs_opt)
        {
            break;
        }

        std::vector<Bridge::ABI::StringView> columns;
        columns.reserve(row_fields.size());
        for (auto const& field : row_fields)
        {
            columns.emplace_back(field);
        }

        out_logs->WriteData(view_index, columns);

        Graphite::Common::Utility::UniqueID const filter_id{filter_id_str};
        Graphite::Common::Utility::UniqueID const highlight_id{highlight_id_str};
        out_logs->WriteMetadata(view_index, filter_id, highlight_id);

        row_fields.clear();
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3
