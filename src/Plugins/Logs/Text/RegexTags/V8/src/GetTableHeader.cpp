/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GetTableHeader.cpp
/// @author Alexandru Delegeanu
/// @version 8.0
/// @brief Implementation @see RegexTags.hpp
///

#include "Fluxion/Plugins/Logs/Text/RegexTags/V8/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8::GetTableHeader);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V8::GetTableHeader);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V8 {

std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> RegexTags::GetTableHeader() const
{
    return m_imported_logs_header;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V8
