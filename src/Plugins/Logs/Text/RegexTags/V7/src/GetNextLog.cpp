/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetNextLog.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetNextLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::GetNextLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

std::optional<std::size_t> RegexTags::GetNextLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index)
{
    LOG_SCOPE("::GetNextLog()");

    if (m_filtered_logs.empty())
    {
        return std::nullopt;
    }

    return SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}.GetNextFilteredIndex(
        m_filtered_logs, filter_id, current_index);
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
