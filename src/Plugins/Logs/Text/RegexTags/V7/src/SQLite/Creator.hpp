/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Wrapper for SQLite create operations
///

#pragma once

#include "Wrapper/DatabaseRef.hpp"

#include <string>
#include <vector>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite {

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

class Creator
{
public: // Lifecycle
    explicit Creator(DatabaseRef db);

    Creator(Creator const&) = delete;
    Creator& operator=(Creator const&) = delete;

    Creator(Creator&&) noexcept = default;
    Creator& operator=(Creator&&) noexcept = default;

public: // Public API
    bool CreateTable(std::vector<std::string> const& fields_ids);

private:
    DatabaseRef m_database;
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite
