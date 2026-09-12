/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 9.8
/// @brief Implementation @see RegexTags.hpp
///

#include <algorithm>
#include <cctype>
#include <future>
#include <memory>
#include <re2/re2.h>
#include <span>
#include <string>
#include <string_view>
#include <variant>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V9/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::ApplyFilters);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V9::ApplyFilters);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

namespace FilterImpl {

inline std::string Lowercase(std::string_view const value)
{
    std::string out{value};
    for (auto& character : out)
    {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return out;
}

inline bool EqualsIgnoreCase(std::string_view const str, std::string_view const str_lowercase)
{
    if (str.size() != str_lowercase.size())
    {
        return false;
    }

    for (std::size_t index{0}; index < str.size(); ++index)
    {
        if (std::tolower(static_cast<unsigned char>(str[index])) != str_lowercase[index])
        {
            return false;
        }
    }
    return true;
}

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

inline std::size_t EvaluationCost(ComputedCondition const& condition)
{
    if (condition[Bridge::EConditionFlag::IsRegex])
    {
        return 2;
    }
    return condition[Bridge::EConditionFlag::IsCaseSensitive] ? 0 : 1;
}

inline std::vector<ActiveFilter> Convert(std::vector<Fluxion::API::LogsPlugin::Bridge::Filter> filters)
{
    LOG_SCOPE("::FilterImpl::Convert()");
    LOG_INFO("::FilterImpl::Convert(): Converting {} filters", filters.size());

    std::vector<ActiveFilter> out{};
    out.reserve(filters.size());

    for (auto const& filter : filters)
    {
        std::vector<ComputedCondition> out_conditions{};
        std::span<Bridge::Condition const> conditions_span{filter.conditions};
        out_conditions.reserve(conditions_span.size());

        for (auto const& condition : conditions_span)
        {
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

            if (condition[Bridge::EConditionFlag::IsRegex])
            {
                out_condition.condition = std::make_unique<re2::RE2>(condition.data, options);
            }
            else
            {
                out_condition.condition = condition[Bridge::EConditionFlag::IsCaseSensitive]
                                              ? std::string(condition.data)
                                              : Lowercase(condition.data);
            }
        }

        std::stable_sort(
            out_conditions.begin(),
            out_conditions.end(),
            [](ComputedCondition const& lhs, ComputedCondition const& rhs) {
                return EvaluationCost(lhs) < EvaluationCost(rhs);
            });

        out.emplace_back(filter.id, filter.priority, std::move(out_conditions));
    }

    return out;
}

} // namespace FilterImpl

void RegexTags::ApplyFiltersABI(
    Bridge::ABI::Span<Bridge::Filter const> const _filters,
    Bridge::ABI::Span<Bridge::Filter const> const _highlight_only)
{
    LOG_SCOPE("::ApplyFiltersABI()");

    std::span<Bridge::Filter const> filters_span{_filters};
    std::span<Bridge::Filter const> highlight_only_span{_highlight_only};

    std::vector<Bridge::Filter> const filters_vector(filters_span.begin(), filters_span.end());
    std::vector<Bridge::Filter> const highlight_vector(
        highlight_only_span.begin(), highlight_only_span.end());

    auto const filters = FilterImpl::Convert(std::move(filters_vector));
    auto const highlight_only = FilterImpl::Convert(std::move(highlight_vector));

    LOG_INFO("::ApplyFiltersABI(): Active filters size: {}", filters.size());
    LOG_INFO("::ApplyFiltersABI(): Highlight-only filters size: {}", highlight_only.size());

    if (m_sqlite_storages.empty())
    {
        LOG_INFO("::ApplyFiltersABI(): No logs were imported, stopping execution");
        return;
    }

    m_logs_operation_target = m_total_logs_imported;
    m_logs_operation_progress = 0;

    {
        LOG_SCOPE("::ApplyFiltersABI(): Execution phase");

        using FilteredLogs = std::vector<Data::FilteredLog>;
        using FilterResult = std::pair<bool, FilteredLogs>;

        std::vector<std::future<FilterResult>> filter_tasks{};
        filter_tasks.reserve(m_sqlite_storages.size());

        for (auto const& storage : m_sqlite_storages)
        {
            filter_tasks.emplace_back(
                std::async(
                    std::launch::async,
                    [this, storage = storage.get(), &filters, &highlight_only]() -> FilterResult {
                        LOG_SCOPE("::ApplyFiltersABI::WorkerThread");
                        FilteredLogs filtered_logs;

                        bool const completed = storage->ReadRowsViews(
                            [this, &filters, &highlight_only, &filtered_logs](
                                std::size_t const log_id, std::vector<std::string_view> const& row) {
                                m_logs_operation_progress.fetch_add(1, std::memory_order_relaxed);

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

                                        bool const equals = [&]() {
                                            if (condition[Bridge::EConditionFlag::IsRegex])
                                            {
                                                auto const* regex_ptr =
                                                    std::get_if<std::unique_ptr<re2::RE2>>(
                                                        &condition.condition);
                                                return regex_ptr && *regex_ptr &&
                                                       re2::RE2::FullMatch(
                                                           re2::StringPiece(
                                                               target.data(), target.size()),
                                                           **regex_ptr);
                                            }
                                            if (condition[Bridge::EConditionFlag::IsCaseSensitive])
                                            {
                                                auto const* str_ptr =
                                                    std::get_if<std::string>(&condition.condition);
                                                return str_ptr && (target == *str_ptr);
                                            }
                                            {
                                                auto const* str_ptr =
                                                    std::get_if<std::string>(&condition.condition);
                                                return str_ptr && FilterImpl::EqualsIgnoreCase(
                                                                      target, *str_ptr);
                                            }
                                        }();

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

                                                bool const equals = [&]() {
                                                    if (condition[Bridge::EConditionFlag::IsRegex])
                                                    {
                                                        auto const* regex_ptr =
                                                            std::get_if<std::unique_ptr<re2::RE2>>(
                                                                &condition.condition);
                                                        return regex_ptr && *regex_ptr &&
                                                               re2::RE2::FullMatch(
                                                                   re2::StringPiece(
                                                                       target.data(), target.size()),
                                                                   **regex_ptr);
                                                    }
                                                    if (condition[Bridge::EConditionFlag::IsCaseSensitive])
                                                    {
                                                        auto const* str_ptr = std::get_if<std::string>(
                                                            &condition.condition);
                                                        return str_ptr && (target == *str_ptr);
                                                    }
                                                    {
                                                        auto const* str_ptr = std::get_if<std::string>(
                                                            &condition.condition);
                                                        return str_ptr && FilterImpl::EqualsIgnoreCase(
                                                                              target, *str_ptr);
                                                    }
                                                }();

                                                if (condition[Bridge::EConditionFlag::IsEquals] !=
                                                    equals)
                                                {
                                                    highlight_matches = false;
                                                    break;
                                                }
                                            }

                                            if (highlight_matches &&
                                                highlight_filter.priority > highlight_priority)
                                            {
                                                highlight_id = highlight_filter.id;
                                                highlight_priority = highlight_filter.priority;
                                            }
                                        }

                                        filtered_logs.emplace_back(log_id, filter.id, highlight_id);
                                        break;
                                    }
                                }
                                return true;
                            });

                        return {completed, std::move(filtered_logs)};
                    }));
        }

        FilteredLogs filtered_logs{};
        for (auto& filter_task : filter_tasks)
        {
            auto [completed, storage_logs] = filter_task.get();
            if (!completed)
            {
                LOG_ERROR("::ApplyFiltersABI(): Filter task execution failed or was interrupted");
                m_logs_operation_progress = 0;
                m_logs_operation_target = 0;
                return;
            }
            filtered_logs.insert(
                filtered_logs.end(),
                std::make_move_iterator(storage_logs.begin()),
                std::make_move_iterator(storage_logs.end()));
        }
        m_filtered_logs = std::move(filtered_logs);
    }

    LOG_INFO("::ApplyFiltersABI(): Total filtered logs matched: {}", m_filtered_logs.size());

    m_logs_operation_progress = 0;
    m_logs_operation_target = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
