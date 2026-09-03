/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.cpp
/// @author Alexandru Delegeanu
/// @version 8.0
/// @brief Data implementation
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V8/Data.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V8::Data {

FilteredLog::FilteredLog(std::size_t const log_id) : log_id{log_id}
{
}

FilteredLog::FilteredLog(
    std::size_t const log_id,
    Graphite::Common::Utility::UniqueID const& filter_id,
    Graphite::Common::Utility::UniqueID const& highlight_filter_id)
    : log_id{log_id}, filter_id{filter_id}, highlight_filter_id{highlight_filter_id}
{
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V8::Data
