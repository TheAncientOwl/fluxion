/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 7.1
/// @brief Implementation @see RegexTags.hpp
///

#include <algorithm>
#include <cctype>
#include <memory>
#include <re2/re2.h>
#include <string>
#include <variant>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "Scrolls/Scribe.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::ApplyFilters);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::ApplyFilters);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

namespace FilterImpl {

using EConditionFlag = Fluxion::API::LogsPlugin::EConditionFlag;

struct ComputedCondition : Graphite::Common::Utility::TWithFlags<ComputedCondition, EConditionFlag>
{
    using TWithFlags<ComputedCondition, EConditionFlag>::operator[];

    std::size_t column_index{};
    std::variant<std::unique_ptr<re2::RE2>, std::string> condition{};
};

struct ActiveFilter
{
    Fluxion::API::LogsPlugin::UniqueID id;
    std::uint8_t priority{};
    std::vector<ComputedCondition> conditions{};
};

inline bool MatchesCondition(ComputedCondition const& condition, std::string_view target)
{
    bool equals = false;

    if (condition[EConditionFlag::IsRegex])
    {
        auto const* re = std::get_if<std::unique_ptr<re2::RE2>>(&condition.condition);
        if (re && *re && (*re)->ok())
        {
            equals = re2::RE2::FullMatch(target, **re);
        }
    }
    else
    {
        auto const* str = std::get_if<std::string>(&condition.condition);
        if (str)
        {
            if (!condition[EConditionFlag::IsCaseSensitive])
            {
                equals = std::equal(
                    target.begin(), target.end(), str->begin(), str->end(), [](char a, char b) {
                        return std::tolower(static_cast<unsigned char>(a)) ==
                               std::tolower(static_cast<unsigned char>(b));
                    });
            }
            else
            {
                equals = (target == *str);
            }
        }
    }

    return condition[EConditionFlag::IsEquals] == equals;
}

inline bool MatchesFilter(ActiveFilter const& filter, Scrolls::Papyrus::Line const& row)
{
    for (auto const& condition : filter.conditions)
    {
        if (condition.column_index >= row.size())
        {
            return false;
        }

        if (!MatchesCondition(condition, row[condition.column_index]))
        {
            return false;
        }
    }
    return true;
}

///
/// @note Conversion has to be done because of plugin specific regex implementation
/// TODO: Consider moving this on Fluxion side with a callback / template type for regex handling.
///
inline std::vector<ActiveFilter> Convert(std::span<Fluxion::API::LogsPlugin::Filter const> const filters)
{
    LOG_SCOPE("::Convert()");
    LOG_INFO("::FilterImpl::Convert(): SIZE: {}", filters.size());

    std::vector<ActiveFilter> out{};
    out.reserve(filters.size());

    for (auto const& filter : filters)
    {
        std::vector<ComputedCondition> out_conditions{};
        out_conditions.reserve(filter.conditions.size());

        for (auto const& condition : filter.conditions)
        {
            auto& out_condition = out_conditions.emplace_back();
            out_condition.column_index = condition.column_index;

            out_condition[EConditionFlag::IsRegex] = condition[EConditionFlag::IsRegex];
            out_condition[EConditionFlag::IsEquals] = condition[EConditionFlag::IsEquals];
            out_condition[EConditionFlag::IsCaseSensitive] =
                condition[EConditionFlag::IsCaseSensitive];

            if (condition[EConditionFlag::IsRegex])
            {
                re2::RE2::Options options;
                options.set_case_sensitive(condition[EConditionFlag::IsCaseSensitive]);

                auto re = std::make_unique<re2::RE2>(condition.data, options);
                if (!re->ok())
                {
                    LOG_ERROR(
                        "::FilterImpl::Convert(): Invalid RE2 pattern '{}': {}",
                        condition.data,
                        re->error());
                }
                out_condition.condition = std::move(re);
            }
            else
            {
                out_condition.condition = condition.data;
            }
        }

        out.emplace_back(filter.id, filter.priority, std::move(out_conditions));
    }

    return out;
}

} // namespace FilterImpl

void RegexTags::ApplyFilters(
    std::span<Fluxion::API::LogsPlugin::Filter const> const _filters,
    std::span<Fluxion::API::LogsPlugin::Filter const> const _highlight_only)
{
    LOG_SCOPE("::ApplyFilters()");

    auto const filters = FilterImpl::Convert(_filters);
    auto const highlight_only = FilterImpl::Convert(_highlight_only);
    LOG_INFO("::ApplyFilters(): Active filters size: {}", filters.size());
    LOG_INFO("::ApplyFilters(): HighlightOnly-Active filters size: {}", highlight_only.size());

    m_filtered_logs.clear();

    if (m_scrolls.GetTotalLinesCount() == 0)
    {
        LOG_INFO("::ApplyFilters(): No logs were imported, stopping execution");
        return;
    }

    auto cursor = m_scrolls.GetCursor();
    std::size_t log_id{0};
    Scrolls::Papyrus::Line row{};

    m_logs_operation_target = m_scrolls.GetTotalLinesCount();
    m_logs_operation_progress = 0;

    {
        LOG_SCOPE("::ApplyFilters(): filtering");

        while (cursor.HasNext())
        {
            if (!cursor.ReadNext(row))
            {
                ++log_id;
                continue;
            }

            ++m_logs_operation_progress;
            for (auto const& filter : filters)
            {
                if (FilterImpl::MatchesFilter(filter, row))
                {
                    Fluxion::API::LogsPlugin::UniqueID highlight_id{filter.id};
                    auto highlight_priority{filter.priority};

                    for (auto const& highlight_filter : highlight_only)
                    {
                        if (highlight_filter.priority > highlight_priority &&
                            FilterImpl::MatchesFilter(highlight_filter, row))
                        {
                            highlight_id = highlight_filter.id;
                            highlight_priority = highlight_filter.priority;
                        }
                    }

                    m_filtered_logs.emplace_back(log_id, filter.id, highlight_id);
                    break;
                }
            }
            ++log_id;
        }
    }

    LOG_INFO("::ApplyFilters(): Total filtered logs: {}", m_filtered_logs.size());

    m_logs_operation_progress = 0;
    m_logs_operation_target = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
