/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 7.6
/// @brief Implementation @see RegexTags.hpp
///

#include <memory>
#include <re2/re2.h>
#include <span>
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

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::Bridge::EConditionFlag>
{
    using TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::Bridge::EConditionFlag>::operator[];

    std::size_t column_index{};
    std::variant<std::unique_ptr<re2::RE2>, std::string> condition{};
};

struct ActiveFilter
{
    Graphite::Common::Utility::UniqueID id;
    std::uint8_t priority{};
    std::vector<ComputedCondition> conditions{};
};

///
/// @note Conversion has to be done because of plugin specific regex implementation
/// TODO: Consider moving this on Fluxion side with a callback / template type for regex handling.
///
inline std::vector<ActiveFilter> Convert(std::span<Bridge::Filter const> const filters)
{
    LOG_SCOPE("::Convert()");
    LOG_INFO("::FilterImpl::Convert(): SIZE: {}", filters.size());

    std::vector<ActiveFilter> out{};
    out.reserve(filters.size());

    for (std::size_t i = 0; i < filters.size(); ++i)
    {
        auto const& filter = filters[i];

        std::vector<ComputedCondition> out_conditions{};
        std::span<Bridge::Condition const> const conditions_span{filter.conditions};
        out_conditions.reserve(conditions_span.size());

        for (std::size_t j = 0; j < conditions_span.size(); ++j)
        {
            auto const& condition = conditions_span[j];

            auto& out_condition = out_conditions.emplace_back();
            out_condition.column_index = condition.column_index;

            out_condition[Bridge::EConditionFlag::IsRegex] =
                condition[Bridge::EConditionFlag::IsRegex];
            out_condition[Bridge::EConditionFlag::IsEquals] =
                condition[Bridge::EConditionFlag::IsEquals];
            out_condition[Bridge::EConditionFlag::IsCaseSensitive] =
                condition[Bridge::EConditionFlag::IsCaseSensitive];

            re2::RE2::Options options;
            options.set_case_sensitive(condition[Bridge::EConditionFlag::IsCaseSensitive]);

            std::string_view const condition_data{condition.data};
            if (condition[Bridge::EConditionFlag::IsRegex])
            {
                out_condition.condition = std::make_unique<re2::RE2>(condition_data, options);
            }
            else
            {
                out_condition.condition = std::string(condition_data);
            }
        }

        out.emplace_back(filter.id, filter.priority, std::move(out_conditions));
    }

    return out;
}

}; // namespace FilterImpl

void RegexTags::ApplyFiltersABI(
    Bridge::ABI::Span<Bridge::Filter const> const _filters,
    Bridge::ABI::Span<Bridge::Filter const> const _highlight_only)
{
    LOG_SCOPE("::ApplyFiltersABI()");

    auto const filters = FilterImpl::Convert(_filters);
    auto const highlight_only = FilterImpl::Convert(_highlight_only);
    LOG_INFO("::ApplyFiltersABI(): Active filters size: {}", filters.size());
    LOG_INFO("::ApplyFiltersABI(): HighlightOnly-Active filters size: {}", highlight_only.size());

    m_filtered_logs.clear();

    if (m_scrolls.GetTotalLinesCount() == 0)
    {
        LOG_INFO("::ApplyFiltersABI(): No logs were imported, stopping execution");
        return;
    }

    auto cursor = m_scrolls.GetCursor();
    std::size_t log_id{0};
    Scrolls::Papyrus::Line row{};

    m_logs_operation_target = m_scrolls.GetTotalLinesCount();
    m_logs_operation_progress = 0;

    {
        LOG_SCOPE("::ApplyFiltersABI(): filtering");

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
                bool matches{true};
                for (auto const& condition : filter.conditions)
                {
                    if (condition.column_index >= row.size())
                    {
                        matches = false;
                        break;
                    }
                    auto const& target{row[condition.column_index]};

                    bool const equals =
                        condition[Bridge::EConditionFlag::IsRegex]
                            ? (std::get<std::unique_ptr<re2::RE2>>(condition.condition) &&
                               re2::RE2::FullMatch(
                                   target, *std::get<std::unique_ptr<re2::RE2>>(condition.condition)))
                            : (target == std::get<std::string>(condition.condition));

                    if (condition[Bridge::EConditionFlag::IsEquals] != equals)
                    {
                        matches = false;
                        break;
                    }
                }

                if (matches)
                {
                    Graphite::Common::Utility::UniqueID highlight_id{filter.id};
                    auto highlight_priority{filter.priority};
                    for (auto const& highlight_filter : highlight_only)
                    {
                        bool highlight_matches{true};
                        for (auto const& condition : highlight_filter.conditions)
                        {
                            if (condition.column_index >= row.size())
                            {
                                highlight_matches = false;
                                break;
                            }
                            auto const& target{row[condition.column_index]};

                            bool const equals =
                                condition[Bridge::EConditionFlag::IsRegex]
                                    ? (std::get<std::unique_ptr<re2::RE2>>(condition.condition) &&
                                       re2::RE2::FullMatch(
                                           target,
                                           *std::get<std::unique_ptr<re2::RE2>>(condition.condition)))
                                    : (target == std::get<std::string>(condition.condition));

                            if (condition[Bridge::EConditionFlag::IsEquals] != equals)
                            {
                                highlight_matches = false;
                                break;
                            }
                        }
                        if (highlight_matches && highlight_filter.priority > highlight_priority)
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

    LOG_INFO("::ApplyFiltersABI(): Total filtered logs: {}", m_filtered_logs.size());

    m_logs_operation_progress = 0;
    m_logs_operation_target = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7
