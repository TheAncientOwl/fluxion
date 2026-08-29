/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.hpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief Wrapper for SQLite create operations
///

#pragma once

#include "Wrapper/DatabaseRef.hpp"

#include <string>
#include <vector>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite {

// --------------------------------------------------
// Tables schemas
// --------------------------------------------------
// [logs]
// id                | number | PK
// field_<field-id0> | string
// field_<field-id1> | string
// ...
// field_<field-idN> | string
// --------------------------------------------------
// [filtered_logs]
// view_index         | number | PK
// log_id             | number | FK references logs:id
// filter_id          | string
// highligh_filter_id | string
// --------------------------------------------------

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

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V6::SQLite
