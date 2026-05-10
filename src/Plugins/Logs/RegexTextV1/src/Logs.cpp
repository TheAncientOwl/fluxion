/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logs.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
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

void RegexTextV1LogsPlugin::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("::ImportLogs()");
    LOG_INFO("Importing {}", path.c_str());

    m_regex_tags.SyncFrontBufferCopy();
    auto const& tags{m_regex_tags.GetFront()};

    std::string full_pattern{};
    m_imported_logs_header.clear();
    for (auto const& tag : tags)
    {
        if (tag->visible)
        {
            // TODO: save to config
            m_imported_logs_header.push_back({tag->id, tag->display_name});
            full_pattern += "(" + tag->regex_data + ")";
        }
        else
        {
            full_pattern += tag->regex_data;
        }
    }
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

    auto const output_path{MakeConvertedLogsPath(path)};
    auto writer = miocsv::Writer{output_path};
    LOG_INFO("Output CSV file {}", output_path.string());

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
            miocsv::Row row{};
            for (std::size_t i = 1; i < matches.size(); ++i)
            {
                if (matches[i].matched)
                {
                    row.append(matches[i].str());
                    // LOG_TRACE("::ImportLogs(): Col {}: {}", i, matches[i].str());
                    // TODO: store or process column value
                    // e.g. LOG_TRACE("Col {}: {}", i, matches[i].str());
                }
            }
            writer.write_row(row);
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

    auto reader = miocsv::MIOReader{MakeConvertedLogsPath(*m_last_imported_logs_path)};
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
        if (target_row.data.size() < row.size())
        {
            target_row.data.resize(row.size());
        }

        for (std::size_t col_idx = 0; col_idx < row.size(); ++col_idx)
        {
            // TODO: check with move(row)
            target_row.data[col_idx] = row[col_idx];
        }

        // TODO: update with filters when implemented
        target_row.metadata = {};
    }
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1
