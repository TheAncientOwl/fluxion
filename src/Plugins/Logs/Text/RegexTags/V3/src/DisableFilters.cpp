/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DisableFilters.cpp
/// @author Alexandru Delegeanu
/// @version 3.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V3/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/QueryExecutor.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3 {

void RegexTags::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
    auto settings{GetConfig()};
    auto const total_logs_imported_opt{settings.get<std::size_t>("total_logs_imported")};
    if (!static_cast<bool>(total_logs_imported_opt))
    {
        LOG_WARN("::DisableFilters(): total_logs_imported is not set in config");
        return;
    }

    settings.set("total_logs", *total_logs_imported_opt);
    settings.Save();

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::DisableFilters(): No logs imported before, nothing to disable");
        return;
    }

    std::error_code ec;
    auto const db_path = MakeDatabasePath(*m_last_imported_logs_path);
    if (std::filesystem::file_size(db_path, ec) == 0 || ec)
    {
        LOG_WARN("::DisableFilters(): Database file {} is 0 bytes or locked.", db_path.string());
        return;
    }

    SQLite::QueryExecutor executor{db_path};

    if (!executor.OpenDatabase())
    {
        return;
    }

    m_logs_progress = 0;

    executor.AddProgressHandler(1000, [this, &executor]() {
        m_logs_progress = executor.GetFilteredLogsCount();
        return 0;
    });

    auto const success = executor.ExecuteTransaction([&executor]() {
        return executor.Execute("DROP TABLE IF EXISTS filtered_logs;") &&
               executor.Execute(
                   "CREATE TABLE filtered_logs ("
                   "    view_index INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "    log_id INTEGER NOT NULL,"
                   "    filter_id TEXT,"
                   "    highlight_filter_id TEXT,"
                   "    FOREIGN KEY(log_id) REFERENCES logs(id)"
                   ");") &&
               executor.Execute(
                   "INSERT INTO filtered_logs (view_index, log_id, filter_id, highlight_filter_id) "
                   "SELECT ID, ID, NULL, NULL FROM logs;");
    });

    executor.RemoveProgressHandler();

    if (!success)
    {
        LOG_ERROR("::DisableFilters(): Failed to reset filtered_logs table for disabled filters.");
        return;
    }

    m_logs_progress = 0;
    LOG_INFO(
        "::DisableFilters(): Successfully disabled filters. Restored {} logs.",
        *total_logs_imported_opt);
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3
