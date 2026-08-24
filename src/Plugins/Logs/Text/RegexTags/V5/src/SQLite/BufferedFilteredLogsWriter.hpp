/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedFilteredLogsWriter.hpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Wrapper for buffered writes to the filtered_logs table
///

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Wrapper/DatabaseRef.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite {

class BufferedFilteredLogsWriter
{
public:
    struct FilteredRow
    {
        std::size_t log_id{0};
        std::string filter_id{};
        std::string highlight_filter_id{};
    };

public: // Lifecycle
    BufferedFilteredLogsWriter(DatabaseRef db, std::size_t const batch_size);
    ~BufferedFilteredLogsWriter();

    BufferedFilteredLogsWriter(BufferedFilteredLogsWriter const&) = delete;
    BufferedFilteredLogsWriter& operator=(BufferedFilteredLogsWriter const&) = delete;

    BufferedFilteredLogsWriter(BufferedFilteredLogsWriter&&) noexcept = default;
    BufferedFilteredLogsWriter& operator=(BufferedFilteredLogsWriter&&) noexcept = default;

public: // Public API
    /**
     * @brief Gets the next frame (row buffer) to be populated.
     * @return Reference to the FilteredRow frame struct.
     */
    FilteredRow& NextFrame();

    bool ClearTable();
    bool Flush();

private: // Private API
    bool ExecuteFlush();

private:
    DatabaseRef m_database;
    std::vector<FilteredRow> m_buffer{};
    std::size_t m_batch_size{0};
    std::size_t m_current_index{0};
    std::int64_t m_view_index{-1};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite
