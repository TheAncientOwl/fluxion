/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 6.4
/// @brief Implementation @see RegexTags.hpp
///

#include <memory>
#include <re2/re2.h>
#include <span>
#include <string>
#include <variant>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V6/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "SQLite/LogsReader.hpp"
#include "SQLite/Utility.hpp"
#include "SQLite/Wrapper/Transaction.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::ApplyFilters);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V6::ApplyFilters);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6 {

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

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::ApplyFiltersABI(): No logs were imported, stopping execution");
        return;
    }

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::ApplyFiltersABI(): SQLite connection is closed and could not be opened");
        return;
    }

    auto logs_reader{SQLite::LogsReader{m_sqlite_connection.GetDatabaseRef()}};
    auto statement =
        logs_reader.PrepareGetAllLogsQuery(SQLite::Utility::MakeFieldsIDs(m_imported_logs_header));
    if (!statement.IsValid())
    {
        return;
    }

    std::size_t log_id{0};
    std::vector<std::string> row{};

    m_logs_operation_target = m_total_logs_imported;
    m_logs_operation_progress = 0;
    {
        LOG_SCOPE("::ApplyFiltersABI(): filtering");
        SQLite::Transaction transaction{m_sqlite_connection.GetDatabaseRef()};
        if (!transaction.IsActive())
        {
            LOG_ERROR(
                "::ApplyFiltersABI(): Failed to begin transaction: {}",
                m_sqlite_connection.GetDatabaseRef().GetLastErrorMessage());
            return;
        }
        while (logs_reader.NextRow(statement, log_id, row))
        {
            ++m_logs_operation_progress;
            for (auto const& filter : filters)
            {
                bool matches{true};
                for (auto const& condition : filter.conditions)
                {
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
        }

        if (!transaction.Commit())
        {
            LOG_ERROR(
                "::ApplyFiltersABI(): Failed to commit transaction: {}",
                m_sqlite_connection.GetDatabaseRef().GetLastErrorMessage());
            return;
        }
    }

    LOG_INFO("::ApplyFiltersABI(): Total filtered logs: {}", m_filtered_logs.size());

    m_logs_operation_progress = 0;
    m_logs_operation_target = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6
