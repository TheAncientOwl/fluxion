/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetNextLog.cpp
/// @author Alexandru Delegeanu
/// @version 6.3
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V6/RegexTags.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetNextLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetNextLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6 {

bool RegexTags::GetNextLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetNextLogABI()");

    if (!out_index || m_filtered_logs.empty())
    {
        return false;
    }

    // Wrap around to start if out of bounds or at the end
    if (current_index >= m_filtered_logs.size())
    {
        *out_index = 0;
        return true;
    }

    auto const next_index_opt =
        SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}.GetNextFilteredIndex(
            m_filtered_logs, filter_id, current_index);

    if (next_index_opt.has_value())
    {
        *out_index = *next_index_opt;
        return true;
    }

    // Wrap around to 0 if no subsequent match is found
    *out_index = 0;
    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6
