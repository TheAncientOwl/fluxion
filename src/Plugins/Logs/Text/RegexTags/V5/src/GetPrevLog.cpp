/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetPrevLog.cpp
/// @author Alexandru Delegeanu
/// @version 5.2.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V5/RegexTags.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::GetPrevLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::GetPrevLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5 {

bool RegexTags::GetPrevLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetPrevLogABI()");

    if (!out_index)
    {
        return false;
    }

    if (!m_last_imported_logs_path)
    {
        LOG_INFO("::GetPrevLogABI(): No logs imported");
        return false;
    }

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::GetPrevLogABI(): SQLite connection is closed and could not be opened");
        return false;
    }

    auto prev_index_opt =
        SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}.GetPrevFilteredIndex(
            filter_id.ToString(), current_index);

    if (prev_index_opt.has_value())
    {
        *out_index = *prev_index_opt;
        return true;
    }

    return false;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5
