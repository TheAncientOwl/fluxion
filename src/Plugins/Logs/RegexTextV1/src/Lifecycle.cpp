/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Lifecycle.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <string>

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

void RegexTextV1LogsPlugin::OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data)
{
    using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

    m_home_path = data.plugin_home_path;

    LOG_SCOPE("::OnEnable()");
    LOG_TRACE("::OnEnable()");
    m_regex_tags.UpdateBackBufferCopy([](RegexTags& back_tags) {
        //{ auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
        // new_tag->display_name = "New Tag";
        // new_tag->regex_data = ".*";
        // new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
        // new_tag->visible = true;}

        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Timestamp";
            new_tag->regex_data = R"(\d+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Channel";
            new_tag->regex_data = R"(Channel[1-4])";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Level";
            new_tag->regex_data = R"(trace|info|error|debug|warn)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "-";
            new_tag->regex_data = R"(\s+)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = false;
        }
        {
            auto& new_tag = back_tags.emplace_back(std::make_shared<Data::RegexTag>());
            new_tag->display_name = "Payload";
            new_tag->regex_data = R"(.*)";
            new_tag->id = Graphite::Common::Utility::UniqueID::Generate();
            new_tag->visible = true;
        }
    });
}

void RegexTextV1LogsPlugin::OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/)
{
    LOG_SCOPE("::OnDisable()");
    LOG_TRACE("::OnDisable()");
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1
