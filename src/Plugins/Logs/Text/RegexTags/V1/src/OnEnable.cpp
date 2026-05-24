/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file OnEnable.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Implementation @see RegexTags.hpp
///

#include <string>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

void RegexTags::OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data)
{
    using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

    m_home_path = data.plugin_home_path;

    LOG_SCOPE("::OnEnable()");
    LOG_TRACE("::OnEnable()");

    auto tags{LoadRegexTags()};
    if (tags.empty())
    {
        {
            auto& new_tag = tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Timestamp";
            new_tag->regex_data = R"(\d+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Channel";
            new_tag->regex_data = R"(Channel[1-4])";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Level";
            new_tag->regex_data = R"(trace|info|error|debug|warn)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Payload";
            new_tag->regex_data = R"(.*)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
    }

    m_regex_tags.UpdateBackBufferCopy([this, &tags](RegexTags& back_tags) {
        back_tags = std::move(tags);
        UpdateImportedLogsHeader(back_tags);
        SaveRegexTags(back_tags);
    });
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
