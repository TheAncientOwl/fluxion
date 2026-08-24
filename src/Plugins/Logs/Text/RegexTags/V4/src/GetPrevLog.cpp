/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetPrevLog.cpp
/// @author Alexandru Delegeanu
/// @version 4.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V4/RegexTags.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V4::GetPrevLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V4::GetPrevLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V4 {

std::optional<std::size_t> RegexTags::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t current_index)
{
    LOG_SCOPE("::GetPrevLog()");

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::GetPrevLog(): SQLite connection is closed and could not be opened");
        return std::nullopt;
    }

    return SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}.GetPrevFilteredIndex(
        filter_id.ToString(), current_index);
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V4
