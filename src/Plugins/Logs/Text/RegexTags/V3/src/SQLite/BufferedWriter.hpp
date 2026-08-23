
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BufferedWriter.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Wrapper for SQLite write operations
///

#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>

#include "OpenCloseManager.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

class BufferedWriter : public OpenCloseManager
{
public: // Lifecycle
    BufferedWriter(
        std::filesystem::path db_path,
        std::size_t const batch_size,
        std::vector<std::string> const& fields);
    ~BufferedWriter();

public: // Public API
    std::vector<std::string>& NextFrame();
    bool Flush();

private: // Private API
    bool ExecuteFlush();

private:
    std::vector<std::vector<std::string>> m_buffer{};
    std::size_t m_batch_size{0};
    std::size_t m_current_index{0};
    std::string m_fields{""};
    std::string m_fields_sql_placeholders{""};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
