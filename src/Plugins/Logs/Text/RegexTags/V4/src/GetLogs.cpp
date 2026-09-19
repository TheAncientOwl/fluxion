/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetLogs.cpp
/// @author Alexandru Delegeanu
/// @version 4.1
/// @brief Implementation @see RegexTags.hpp
///

#include <filesystem>
#include <system_error>
#include <vector>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V4/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"
#include "SQLite/Utility.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V4::GetLogs);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V4::GetLogs);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V4 {

void RegexTags::GetLogs(
    std::span<Fluxion::API::LogsPlugin::Range const> const ranges,
    Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
    Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
    void* user_data)
{
    LOG_SCOPE("::GetLogs()");

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::GetLogs(): SQLite connection is closed and could not be opened");
        return;
    }

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogs(): No logs were imported before");
        return;
    }

    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    if (!static_cast<bool>(total_logs_opt))
    {
        LOG_WARN("::GetLogs(): total_logs is not set in config");
        return;
    }

    std::error_code ec;
    auto const db_path = MakeDatabasePath(*m_last_imported_logs_path);
    if (std::filesystem::file_size(db_path, ec) == 0 || ec)
    {
        LOG_WARN(
            "::GetLogs(): Database file {} is currently 0 bytes or locked. Skipping read.",
            db_path.string());
        return;
    }

    if (m_imported_logs_header.empty())
    {
        LOG_WARN("::GetLogs(): m_imported_logs_header is empty.");
        return;
    }

    auto reader{SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}};
    auto query_handle{reader.PrepareGetRangesQuery(
        ranges, SQLite::Utility::MakeFieldsIDs(m_imported_logs_header))};
    if (!query_handle.IsValid())
    {
        LOG_ERROR("::GetLogs(): Failed to prepare ranges query.");
        return;
    }

    std::vector<std::string> row_fields;
    std::string filter_id_str;
    std::string highlight_id_str;
    std::size_t view_index = 0;

    while (reader.NextFilteredRow(query_handle, row_fields, filter_id_str, highlight_id_str, view_index))
    {
        if (view_index > *total_logs_opt)
        {
            break;
        }

        // Prepare Row Data
        std::vector<Fluxion::API::LogsPlugin::LogRowItem> row_data{};
        row_data.reserve(row_fields.size());

        for (auto const& field : row_fields)
        {
            row_data.push_back({.data = field.data(), .size = field.size()});
        }

        Fluxion::API::LogsPlugin::LogRowData const safe_data{
            .data = row_data.data(), .size = row_data.size()};
        GRAPHITE_ASSERT(write_data != nullptr, "Received nullptr write_data function pointer");
        write_data(user_data, view_index, &safe_data);

        GRAPHITE_ASSERT(
            write_metadata != nullptr, "Received nullptr write_metadata function pointer");
        auto const metadata{Fluxion::API::LogsPlugin::Adapter::MakeMetadata(
            Fluxion::API::LogsPlugin::UniqueID(filter_id_str),
            Fluxion::API::LogsPlugin::UniqueID(highlight_id_str))};
        write_metadata(user_data, view_index, &metadata);

        row_fields.clear();
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V4
