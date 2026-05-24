/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Data structs
///

#include <string>

#include "Graphite/Common/Utility/UniqueID.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1::Data {

struct RegexTag
{
    Graphite::Common::Utility::UniqueID id{};
    std::string display_name{};
    std::string regex_data{};
    bool visible{};
};

using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1::Data
