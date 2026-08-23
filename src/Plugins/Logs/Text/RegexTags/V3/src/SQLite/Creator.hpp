/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.hpp
/// @author Alexandru Delegeanu
/// @version 3.1
/// @brief Wrapper for SQLite create operations
///

#pragma once

#include "Wrapper/DatabaseRef.hpp"

#include <string>
#include <vector>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

// Table schema:
// id: Number autoincrement
// field_<field-id>: String
// filter_id: string
// highlight_filter_id: string
// filtered_view_index: number

class Creator
{
public: // Lifecycle
    explicit Creator(DatabaseRef db);

    Creator(Creator const&) = delete;
    Creator& operator=(Creator const&) = delete;

    Creator(Creator&&) noexcept = default;
    Creator& operator=(Creator&&) noexcept = default;

public: // Public API
    bool CreateTables(std::vector<std::string> const& fields_ids);

private:
    DatabaseRef m_database;
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
