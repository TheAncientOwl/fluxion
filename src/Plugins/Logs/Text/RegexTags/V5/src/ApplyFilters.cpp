/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 5.1
/// @brief Implementation @see RegexTags.hpp
///

#include <memory>
#include <re2/re2.h>
#include <string>
#include <variant>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V5/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "SQLite/BufferedFilteredLogsWriter.hpp"
#include "SQLite/LogsReader.hpp"
#include "SQLite/Utility.hpp"
#include "SQLite/Wrapper/Transaction.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::ApplyFilters);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V5::ApplyFilters);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5 {

namespace FilterImpl {

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
                out_condition.condition = std::move(condition.data);
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

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::ApplyFilters(): No logs were imported, stopping execution");
        return;
    }

    if (!m_sqlite_connection.IsOpen() &&
        !m_sqlite_connection.OpenDatabase(MakeDatabasePath(*m_last_imported_logs_path)))
    {
        LOG_WARN("::ApplyFilters(): SQLite connection is closed and could not be opened");
        return;
    }

    auto logs_reader{SQLite::LogsReader{m_sqlite_connection.GetDatabaseRef()}};
    auto statement =
        logs_reader.PrepareGetAllLogsQuery(SQLite::Utility::MakeFieldsIDs(m_imported_logs_header));
    if (!statement.IsValid())
    {
        return;
    }

    auto filtered_logs_writer{
        SQLite::BufferedFilteredLogsWriter{m_sqlite_connection.GetDatabaseRef(), 5000}};
    if (!filtered_logs_writer.ClearTable())
    {
        return;
    }

    std::size_t log_id{0};
    std::vector<std::string> row{};

    std::size_t total_filtered_logs{0};
    m_logs_operation_progress = 0;
    {
        LOG_SCOPE("::ApplyFilters(): filtering");
        SQLite::Transaction transaction{m_sqlite_connection.GetDatabaseRef()};
        if (!transaction.IsActive())
        {
            LOG_ERROR(
                "::ApplyFilters(): Failed to begin transaction: {}",
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
                        condition[EConditionFlag::IsRegex]
                            ? (std::get<std::unique_ptr<re2::RE2>>(condition.condition) &&
                               re2::RE2::FullMatch(
                                   target, *std::get<std::unique_ptr<re2::RE2>>(condition.condition)))
                            : (target == std::get<std::string>(condition.condition));

                    if (condition[EConditionFlag::IsEquals] != equals)
                    {
                        matches = false;
                        break;
                    }
                }

                if (matches)
                {
                    ++total_filtered_logs;

                    Graphite::Common::Utility::UniqueID highlight_id{filter.id};
                    auto highlight_priority{filter.priority};
                    for (auto const& highlight_filter : highlight_only)
                    {
                        bool highlight_matches{true};
                        for (auto const& condition : highlight_filter.conditions)
                        {
                            auto const& target{row[condition.column_index]};

                            bool const equals =
                                condition[EConditionFlag::IsRegex]
                                    ? (std::get<std::unique_ptr<re2::RE2>>(condition.condition) &&
                                       re2::RE2::FullMatch(
                                           target,
                                           *std::get<std::unique_ptr<re2::RE2>>(condition.condition)))
                                    : (target == std::get<std::string>(condition.condition));

                            if (condition[EConditionFlag::IsEquals] != equals)
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

                    auto& frame = filtered_logs_writer.NextFrame();
                    frame.log_id = log_id;
                    frame.filter_id = filter.id.ToString();
                    frame.highlight_filter_id = highlight_id.ToString();
                    break;
                }
            }
        }
        filtered_logs_writer.Flush();

        if (!transaction.Commit())
        {
            LOG_ERROR(
                "::ApplyFilters(): Failed to commit transaction: {}",
                m_sqlite_connection.GetDatabaseRef().GetLastErrorMessage());
            return;
        }
    }

    LOG_INFO("::ApplyFilters(): Total filtered logs: {}", total_filtered_logs);
    auto settings{GetConfig()};
    settings.set("total_logs", total_filtered_logs);
    settings.Save();

    m_logs_operation_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5
