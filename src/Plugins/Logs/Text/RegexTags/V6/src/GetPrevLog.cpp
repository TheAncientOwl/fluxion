/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetPrevLog.cpp
/// @author Alexandru Delegeanu
/// @version 6.3
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V6/RegexTags.hpp"
#include "Graphite/Logger.hpp"
#include "SQLite/FilteredLogsReader.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetPrevLog);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetPrevLog);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6 {

bool RegexTags::GetPrevLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetPrevLogABI()");

    if (!out_index || m_filtered_logs.empty())
    {
        return false;
    }

    // Clamp out-of-bounds or end indices to the last valid log index
    std::size_t const effective_index =
        (current_index >= m_filtered_logs.size()) ? m_filtered_logs.size() - 1 : current_index;

    auto prev_index_opt =
        SQLite::FilteredLogsReader{m_sqlite_connection.GetDatabaseRef()}.GetPrevFilteredIndex(
            m_filtered_logs, filter_id, effective_index);

    if (prev_index_opt.has_value())
    {
        *out_index = *prev_index_opt;
        return true;
    }

    return false;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6
