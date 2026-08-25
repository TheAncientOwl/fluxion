/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsWriter.hpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Direct SQLite writer executing chunk-level transactions without double-buffering
///

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Wrapper/DatabaseRef.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite {

class LogsWriter
{
public: // Lifecycle
    LogsWriter(DatabaseRef db, std::vector<std::string> const& fields);
    ~LogsWriter() = default;

    LogsWriter(LogsWriter const&) = delete;
    LogsWriter& operator=(LogsWriter const&) = delete;

    LogsWriter(LogsWriter&&) noexcept = default;
    LogsWriter& operator=(LogsWriter&&) noexcept = default;

public: // Public API
    bool WriteChunk(std::vector<std::vector<std::string>> const& rows, std::size_t active_rows);

private:
    DatabaseRef m_database;
    std::int64_t m_log_id{-1};
    Statement m_logs_statement{};
    Statement m_filtered_logs_statement{};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite
