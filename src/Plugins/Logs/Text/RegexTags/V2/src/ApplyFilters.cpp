/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 2.4
/// @brief Implementation @see RegexTags.hpp
///

#include <algorithm>
#include <cctype>
#include <memory>
#include <re2/re2.h>
#include <string>
#include <variant>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V2/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V2);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V2 {

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

inline bool MatchesFilter(ActiveFilter const& filter, auto const& row)
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

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::ApplyFilters(): No logs were imported, stopping execution");
        return;
    }
    auto const output_filtered_path{MakeFilteredLogsPath(*m_last_imported_logs_path)};
    auto filtered_logs_writer = CSV::Writer{output_filtered_path};
    LOG_INFO("::ApplyFilters(): Output filtered CSV file {}", output_filtered_path);

    auto const input_logs_path{MakeConvertedLogsPath(*m_last_imported_logs_path)};
    auto converted_logs_reader = CSV::Reader{input_logs_path};
    LOG_INFO("::ApplyFilters(): Converted CSV file {}", input_logs_path);

    std::size_t total_filtered_logs{0};
    m_logs_operation_progress = 0;
    for (auto row : converted_logs_reader)
    {
        ++m_logs_operation_progress;
        for (auto const& filter : filters)
        {
            if (FilterImpl::MatchesFilter(filter, row))
            {
                ++total_filtered_logs;

                Fluxion::API::LogsPlugin::UniqueID highlight_id{filter.id};
                auto highlight_priority{filter.priority};

                for (auto const& highlight_filter : highlight_only)
                {
                    // Priority Short-Circuit: Skip regex evaluation if priority is not higher
                    if (highlight_filter.priority > highlight_priority &&
                        FilterImpl::MatchesFilter(highlight_filter, row))
                    {
                        highlight_id = highlight_filter.id;
                        highlight_priority = highlight_filter.priority;
                    }
                }

                std::vector<std::string> filtered_row;
                filtered_row.reserve(row.size() + 2);
                filtered_row.push_back(filter.id.ToString());
                filtered_row.push_back(highlight_id.ToString());
                for (auto const& field : row)
                {
                    filtered_row.push_back(field);
                }
                filtered_logs_writer.write_row(filtered_row);
                break;
            }
        }
    }

    LOG_INFO("::ApplyFilters(): Total filtered logs: {}", total_filtered_logs);
    auto settings{GetConfig()};
    // TODO: move "total_logs" key to some constexpr global
    settings.set("total_logs", total_filtered_logs);
    settings.Save();

    m_logs_operation_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V2
