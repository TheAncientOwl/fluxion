/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ApplyFilters.cpp
/// @author Alexandru Delegeanu
/// @version 1.2.0
/// @brief Implementation @see RegexTags.hpp
///

#include <regex>
#include <span>
#include <string>
#include <variant>

#include "Fluxion/Plugins/Logs/Text/RegexTags/V1/RegexTags.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

#include "CSV/Wrapper/Wrapper.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V1);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1 {

namespace FilterImpl {

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Bridge::EConditionFlag>
{
    using TWithFlags<ComputedCondition, Bridge::EConditionFlag>::operator[];

    std::size_t column_index{};
    std::variant<std::regex, std::string> condition{};
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
    using namespace Bridge;
    LOG_INFO("::FilterImpl::Convert(): SIZE: {}", filters.size());

    std::vector<ActiveFilter> out{};
    out.reserve(filters.size());

    for (auto const& filter : filters)
    {
        std::vector<ComputedCondition> out_conditions{};

        std::span<Condition const> conditions_span = filter.conditions;
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

            std::string_view const data_sv = condition.data;

            if (condition[Bridge::EConditionFlag::IsRegex])
            {
                out_condition.condition = std::regex{std::string(data_sv)};
            }
            else
            {
                out_condition.condition = std::string(data_sv);
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

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::ApplyFiltersABI(): No logs were imported, stopping execution");
        return;
    }
    auto const output_filtered_path{MakeFilteredLogsPath(*m_last_imported_logs_path)};
    auto filtered_logs_writer = CSV::Writer{output_filtered_path};
    LOG_INFO("::ApplyFiltersABI(): Output filtered CSV file {}", output_filtered_path);

    auto const input_logs_path{MakeConvertedLogsPath(*m_last_imported_logs_path)};
    auto converted_logs_reader = CSV::Reader{input_logs_path};
    LOG_INFO("::ApplyFiltersABI(): Converted CSV file {}", input_logs_path);

    std::size_t total_filtered_logs{0};
    m_logs_operation_progress = 0;
    for (auto row : converted_logs_reader)
    {
        ++m_logs_operation_progress;
        for (auto const& filter : filters)
        {
            bool matches{true};
            for (auto const& condition : filter.conditions)
            {
                auto const& target{row[condition.column_index]};

                bool const equals{
                    condition[Bridge::EConditionFlag::IsRegex]
                        ? std::regex_match(target, std::get<std::regex>(condition.condition))
                        : target == std::get<std::string>(condition.condition)};

                if (condition[Bridge::EConditionFlag::IsEquals] != equals)
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

                        bool const equals{
                            condition[Bridge::EConditionFlag::IsRegex]
                                ? std::regex_match(target, std::get<std::regex>(condition.condition))
                                : target == std::get<std::string>(condition.condition)};

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

    LOG_INFO("::ApplyFiltersABI(): Total filtered logs: {}", total_filtered_logs);
    auto settings{GetConfig()};
    // TODO: move "total_logs" key to some constexpr global
    settings.set("total_logs", total_filtered_logs);
    settings.Save();

    m_logs_operation_progress = 0;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1
