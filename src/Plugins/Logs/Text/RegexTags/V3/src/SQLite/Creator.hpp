
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Creator.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Wrapper for SQLite create operations
///

#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>

#include "OpenCloseManager.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

// Table schema:
// id: Number autoincrement
// field_<field-id>: String
// filter_id: string
// highligh_filter_id: string
// filtered_view_index: number

class Creator : public OpenCloseManager
{
public: // Lifecycle
    Creator(std::filesystem::path db_path);

public: // public API
    bool CreateTable(std::vector<std::string> const& fields_ids);
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
