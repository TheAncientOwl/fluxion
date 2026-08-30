
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Utilityy.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Utilities
///

#include "Utility.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::Utility);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::Utility);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::Utility {

[[nodiscard]] std::vector<std::string> MakeFieldsIDs(
    std::vector<std::shared_ptr<Data::RegexTag>> const& tags)
{
    LOG_SCOPE("::MakeFieldsIDs(vector<shared_ptr<RegexTag>>)");
    std::vector<std::string> fields_ids{};
    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            fields_ids.push_back("field_" + tag->id.ToRawString());
        }
    }
    return fields_ids;
}

[[nodiscard]] std::vector<std::string> MakeFieldsIDs(
    std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> const& columns)
{
    LOG_SCOPE("::MakeFieldsIDs(vector<ColumnDetails>)");
    std::vector<std::string> fields_ids{};
    for (auto const& column : columns)
    {
        fields_ids.push_back("field_" + column.id.ToRawString());
    }
    return fields_ids;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::Utility
