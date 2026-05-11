/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logs.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <exception>
#include <fstream>
#include <regex>
#include <string>

#include "miocsv.h"
#include "stdcsv.h"

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

namespace FilterImpl {

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::Data::EConditionFlag>
{
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
inline std::vector<ActiveFilter> Convert(std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters)
{
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

            if (condition[EConditionFlag::IsRegex])
            {
                out_condition.condition = std::regex{condition.data};
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

void RegexTextV1LogsPlugin::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("::ImportLogs()");
    LOG_INFO("Importing {}", path.c_str());

    m_regex_tags.SyncFrontBufferCopy();
    auto const& tags{m_regex_tags.GetFront()};

    std::string full_pattern{};
    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            m_imported_logs_header.push_back({tag->id, tag->display_name});
            full_pattern += "(" + tag->regex_data + ")";
        }
        else
        {
            full_pattern += tag->regex_data;
        }
    }
    UpdateImportedLogsHeader(tags);
    LOG_INFO("::ImportLogs(): Full regex pattern: {}", full_pattern);

    std::regex line_regex{};
    try
    {
        line_regex = std::regex(full_pattern);
    }
    catch (std::exception const& e)
    {
        LOG_WARN("Invalid regex: {}", e.what());
        return;
    }

    m_last_imported_logs_path = path;
    std::ifstream raw_logs_file{path};
    if (!raw_logs_file.is_open())
    {
        LOG_WARN("::ImportLogs(): Could not open file {}", path.c_str());
        return;
    }

    auto const output_converted_path{MakeConvertedLogsPath(path)};
    auto converted_writer = miocsv::Writer{output_converted_path};
    LOG_INFO("Output converted CSV file {}", output_converted_path.string());

    auto const output_filtered_path{MakeFilteredLogsPath(path)};
    auto filtered_writer = miocsv::Writer{output_filtered_path};
    LOG_INFO("Output filtered CSV file {}", output_filtered_path.string());
    auto const default_filter_id{Graphite::Common::Utility::UniqueID::Default().ToString()};

    std::string line{};
    line.reserve(1024);
    std::size_t total_logs{0};

    while (std::getline(raw_logs_file, line))
    {
        std::smatch matches;
        if (std::regex_match(line, matches, line_regex))
        {
            ++total_logs;
            if (total_logs % 1000 == 0)
            {
                LOG_INFO("Read another 1000 chunk, total: {}", total_logs);
            }

            // matches[0] is full match, start from 1 like Rust version
            // TODO: try and move row outside while
            miocsv::Row row{};
            miocsv::Row filtered_row{default_filter_id, default_filter_id};
            for (std::size_t i = 1; i < matches.size(); ++i)
            {
                if (matches[i].matched)
                {
                    auto match{matches[i].str()};
                    row.append(match);
                    filtered_row.append(std::move(match));
                    // LOG_TRACE("::ImportLogs(): Col {}: {}", i, matches[i].str());
                    // TODO: store or process column value
                }
            }
            converted_writer.write_row(row);
            filtered_writer.write_row(filtered_row);
        }
        else
        {
            LOG_WARN("::ImportLogs(): Regex did not match entry '{}'", line);
        }
    }

    LOG_INFO("::ImportLogs(): Total matched logs: {}", total_logs);
    auto settings{GetConfig()};
    settings.set("total_logs", total_logs);
}

std::optional<std::size_t> RegexTextV1LogsPlugin::GetNextLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetNextLog()");
    return std::nullopt;
}

std::optional<std::size_t> RegexTextV1LogsPlugin::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& /*filter_id*/)
{
    LOG_SCOPE("::GetPrevLog()");
    return std::nullopt;
}

std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> RegexTextV1LogsPlugin::GetTableHeader() const
{
    return m_imported_logs_header;
}

std::size_t RegexTextV1LogsPlugin::GetTotalLogs() const
{
    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    return static_cast<bool>(total_logs_opt) ? *total_logs_opt : 0;
}

void RegexTextV1LogsPlugin::GetLogs(
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) const
{
    LOG_SCOPE("::GetLogs()");

    std::stringstream ss{};
    for (auto range : ranges)
    {
        ss << "[" << range.begin << ", " << range.end << "), ";
    }
    LOG_INFO("::GetLogs(): Requested ranges: {}", ss.str());

    if (!static_cast<bool>(m_last_imported_logs_path))
    {
        LOG_INFO("::GetLogs(): No logs were imported before");
        return;
    }

    auto const total_logs_opt{GetConfig().get<std::size_t>("total_logs")};
    if (!static_cast<bool>(total_logs_opt))
    {
        LOG_WARN("::GetLogs(): total_logs is not set in config");
    }

    auto const last_line_index{[&ranges]() {
        std::size_t last_idx{std::numeric_limits<std::size_t>::min()};
        for (auto& range : ranges)
        {
            last_idx = std::max(last_idx, range.end);
        }
        return last_idx;
    }()};

    auto reader = miocsv::MIOReader{MakeFilteredLogsPath(*m_last_imported_logs_path)};
    for (auto row : reader)
    {
        auto const row_num{reader.get_row_num() - 1};

        if (row_num > last_line_index || row_num > *total_logs_opt)
        {
            break;
        }

        if (!std::any_of(ranges.begin(), ranges.end(), [row_num](auto const& range) {
                return range.begin <= row_num && row_num < range.end;
            }))
        {
            continue;
        }

        auto& target_row = out_logs[row_num];
        auto const actual_row_size{row.size() - 2}; // -2 = first 2 filter IDs
        if (target_row.data.size() != actual_row_size)
        {
            target_row.data.resize(actual_row_size);
        }

        for (std::size_t col_idx = 2; col_idx < row.size(); ++col_idx)
        {
            // TODO: check with move(row)
            target_row.data[col_idx - 2] = row[col_idx];
        }

        target_row.metadata = {
            .filter_id = Graphite::Common::Utility::UniqueID{row[0]},
            .highlight_id = Graphite::Common::Utility::UniqueID{row[1]}};
    }
}

void RegexTextV1LogsPlugin::ApplyFilters(
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
    auto const output_filtered_path{MakeFilteredLogsPath(*m_last_imported_logs_path)};
    auto filtered_logs_writer = miocsv::Writer{output_filtered_path};
    LOG_INFO("::ApplyFilters(): Output filtered CSV file {}", output_filtered_path.string());

    auto const input_logs_path{MakeConvertedLogsPath(*m_last_imported_logs_path)};
    auto converted_logs_reader = miocsv::MIOReader{input_logs_path};
    LOG_INFO("::ApplyFilters(): Converted CSV file {}", input_logs_path.string());

    std::size_t total_filtered_logs{0};
    for (auto row : converted_logs_reader)
    {
        for (auto const& filter : filters)
        {
            bool matches{true};
            for (auto const& condition : filter.conditions)
            {
                auto const& target{row[condition.column_index]};

                bool const equals{
                    condition[EConditionFlag::IsRegex]
                        ? std::regex_match(target, std::get<std::regex>(condition.condition))
                        : target == std::get<std::string>(condition.condition)};

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

                        bool const equals{
                            condition[EConditionFlag::IsRegex]
                                ? std::regex_match(target, std::get<std::regex>(condition.condition))
                                : target == std::get<std::string>(condition.condition)};

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

                auto filtered_row = miocsv::Row{filter.id.ToString(), highlight_id.ToString()};
                for (auto field : row)
                {
                    filtered_row.append(field);
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
}

void RegexTextV1LogsPlugin::DisableFilters()
{
    LOG_SCOPE("::DisableFilters()");
    if (static_cast<bool>(m_last_imported_logs_path))
    {
        ImportLogs(*m_last_imported_logs_path);
    }
    else
    {
        LOG_INFO("::DisableFilters(): No logs imported before, nothing to disable");
    }
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1
