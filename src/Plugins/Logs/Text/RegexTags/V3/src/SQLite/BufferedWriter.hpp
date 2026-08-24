/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedWriter.hpp
/// @author Alexandru Delegeanu
/// @version 3.3
/// @brief Wrapper for SQLite write operations
///

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Wrapper/DatabaseRef.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

class BufferedWriter
{
public: // Lifecycle
    BufferedWriter(DatabaseRef db, std::size_t const batch_size, std::vector<std::string> const& fields);
    ~BufferedWriter();

    BufferedWriter(BufferedWriter const&) = delete;
    BufferedWriter& operator=(BufferedWriter const&) = delete;

    BufferedWriter(BufferedWriter&&) noexcept = default;
    BufferedWriter& operator=(BufferedWriter&&) noexcept = default;

public: // Public API
    std::vector<std::string>& NextFrame();
    bool Flush();

private: // Private API
    bool ExecuteFlush();

private:
    DatabaseRef m_database;
    std::vector<std::vector<std::string>> m_buffer{};
    std::size_t m_batch_size{0};
    std::size_t m_current_index{0};
    std::int64_t m_log_id{-1};
    Statement m_logs_statement{};
    Statement m_filtered_logs_statement{};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
