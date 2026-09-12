/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetNextLog.cpp
/// @author Alexandru Delegeanu
/// @version 3.2.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V3/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::GetNextLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V3::GetNextLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3 {

bool RegexTags::GetNextLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetNextLogABI()");

    if (!out_index)
    {
        return false;
    }

    if (!m_last_imported_logs_path)
    {
        LOG_INFO("::GetNextLogABI(): No logs imported");
        return false;
    }

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::GetNextLogABI(): SQLite connection is closed and could not be opened");
        return false;
    }

    auto next_index_opt =
        SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}.GetNextFilteredIndex(
            filter_id.ToString(), current_index);

    if (next_index_opt.has_value())
    {
        *out_index = *next_index_opt;
        return true;
    }

    return false;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3
