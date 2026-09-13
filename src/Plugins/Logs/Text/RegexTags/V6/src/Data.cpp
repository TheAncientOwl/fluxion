/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.cpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Implementation of @see Data.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V6/Data.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6 {

Data::FilteredLog::FilteredLog(std::size_t const log_id) : log_id{log_id} {};
Data::FilteredLog::FilteredLog(
    std::size_t const log_id,
    Graphite::Common::Utility::UniqueID const& filter_id,
    Graphite::Common::Utility::UniqueID const& highlight_filter_id)
    : log_id{log_id}, filter_id{filter_id}, highlight_filter_id{highlight_filter_id}
{
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6
