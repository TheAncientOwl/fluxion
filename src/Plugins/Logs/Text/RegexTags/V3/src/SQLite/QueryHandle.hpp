/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file QueryHandle.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Wrapper for SQLite query handle
///

#pragma once

#include <sqlite3.h>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

class QueryHandle
{
public:
    QueryHandle() = default;
    explicit QueryHandle(sqlite3_stmt* stmt);
    ~QueryHandle();

    QueryHandle(QueryHandle const&) = delete;
    QueryHandle& operator=(QueryHandle const&) = delete;

    QueryHandle(QueryHandle&& other) noexcept;
    QueryHandle& operator=(QueryHandle&& other) noexcept;

    explicit operator bool() const;
    sqlite3_stmt* Get() const;

    void Finalize();

private:
    sqlite3_stmt* m_stmt{nullptr};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
