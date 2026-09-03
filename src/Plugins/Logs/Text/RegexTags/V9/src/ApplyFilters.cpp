/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 9.2
/// @brief Implementation @see RegexTags.hpp
///

#include <cctype>
#include <future>
#include <memory>
#include <re2/re2.h>
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

inline std::string Lowercase(std::string_view value)
{
    std::string out{value};
    for (auto& character : out)
    {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return out;
}

inline bool equalsIgnoreCase(std::string_view const str, std::string_view const str_lowercase)
{
    if (str.size() != str_lowercase.size())
    {
        return false;
    }

    for (std::size_t index{0}; index < str.size(); ++index)
    {
        if (std::tolower(str[index]) != str_lowercase[index])
        {
            return false;
        }
    }
    return true;
}

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::Data::EConditionFlag>
{
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
inline std::vector<ActiveFilter> Convert(std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters)
{
    LOG_SCOPE("::Convert()");
    using namespace Fluxion::API::LogsPlugin::Data;
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

            re2::RE2::Options options;
            options.set_case_sensitive(condition[EConditionFlag::IsCaseSensitive]);

            if (condition[EConditionFlag::IsRegex])
            {
                out_condition.condition = std::make_unique<re2::RE2>(condition.data, options);
            }
            else
            {
                out_condition.condition = condition[EConditionFlag::IsCaseSensitive]
                                              ? std::move(condition.data)
                                              : Lowercase(condition.data);
            }
        }

        out.emplace_back(filter.id, filter.priority, std::move(out_conditions));
    }

    return out;
}

}; // namespace FilterImpl

void RegexTags::ApplyFilters(
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> _filters,
    std::vector<Fluxion::API::LogsPlugin::Data::Filter> _highlight_only)
{
    LOG_SCOPE("::ApplyFilters()");
    using namespace Fluxion::API::LogsPlugin::Data;

    auto const filters = FilterImpl::Convert(std::move(_filters));
    auto const highlight_only = FilterImpl::Convert(std::move(_highlight_only));
    LOG_INFO("::ApplyFilters(): Active filters size: {}", filters.size());
    LOG_INFO("::ApplyFilters(): HighlightOnly-Active filters size: {}", highlight_only.size());

    if (m_sqlite_storages.empty())
    {
        LOG_INFO("::ApplyFilters(): No logs were imported, stopping execution");
        return;
    }

    m_logs_operation_target = m_total_logs_imported;
    m_logs_operation_progress = 0;

    {
        LOG_SCOPE("::ApplyFilters(): filtering");

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
                        LOG_SCOPE("::ApplyFilters::Thread::{}()", std::this_thread::get_id());
                        FilteredLogs filtered_logs;
                        bool const completed = storage->ReadRows(
                            [this, &filters, &highlight_only, &filtered_logs](
                                std::size_t const log_id, std::vector<std::string> const& row) {
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
                                            condition[EConditionFlag::IsRegex]
                                                ? (std::get<std::unique_ptr<re2::RE2>>(
                                                       condition.condition) &&
                                                   re2::RE2::FullMatch(
                                                       target,
                                                       *std::get<std::unique_ptr<re2::RE2>>(
                                                           condition.condition)))
                                            : condition[EConditionFlag::IsCaseSensitive]
                                                ? (target == std::get<std::string>(condition.condition))
                                                : FilterImpl::equalsIgnoreCase(
                                                      target,
                                                      std::get<std::string>(condition.condition));

                                        if (condition[EConditionFlag::IsEquals] != equals)
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
                                                    condition[EConditionFlag::IsRegex]
                                                        ? (std::get<std::unique_ptr<re2::RE2>>(
                                                               condition.condition) &&
                                                           re2::RE2::FullMatch(
                                                               target,
                                                               *std::get<std::unique_ptr<re2::RE2>>(
                                                                   condition.condition)))
                                                    : condition[EConditionFlag::IsCaseSensitive]
                                                        ? (target ==
                                                           std::get<std::string>(condition.condition))
                                                        : FilterImpl::equalsIgnoreCase(
                                                              target,
                                                              std::get<std::string>(
                                                                  condition.condition));

                                                if (condition[EConditionFlag::IsEquals] != equals)
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
        filtered_logs.reserve(m_total_logs_imported);
        for (auto& filter_task : filter_tasks)
        {
            auto [completed, storage_logs] = filter_task.get();
            if (!completed)
            {
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

    LOG_INFO("::ApplyFilters(): Total filtered logs: {}", m_filtered_logs.size());

    m_logs_operation_progress = 0;
    m_logs_operation_target = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9
