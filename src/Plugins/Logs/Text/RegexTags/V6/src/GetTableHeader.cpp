/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetTableHeader.cpp
/// @author Alexandru Delegeanu
/// @version 6.1
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V6/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetTableHeader);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::GetTableHeader);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6 {

Bridge::ABI::Span<Bridge::ColumnDetails> RegexTags::GetTableHeaderABI() const
{
    LOG_SCOPE("::GetTableHeaderABI()");
    return m_imported_logs_header;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6
