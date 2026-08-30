/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Statement.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation of @see Statement.hpp
///

#include "Statement.hpp"

#include <sqlite3.h>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite {

Statement::Statement(sqlite3_stmt* stmt) : m_stmt{stmt}
{
}

Statement::~Statement()
{
    if (m_stmt)
    {
        sqlite3_finalize(m_stmt);
    }
}

Statement::Statement(Statement&& other) noexcept : m_stmt{other.m_stmt}
{
    other.m_stmt = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept
{
    if (this != &other)
    {
        if (m_stmt)
        {
            sqlite3_finalize(m_stmt);
        }
        m_stmt = other.m_stmt;
        other.m_stmt = nullptr;
    }
    return *this;
}

bool Statement::IsValid() const
{
    return m_stmt != nullptr;
}

bool Statement::BindInt64(int index, std::int64_t value)
{
    return sqlite3_bind_int64(m_stmt, index, static_cast<sqlite3_int64>(value)) == SQLITE_OK;
}

bool Statement::BindText(int index, std::string_view value)
{
    return sqlite3_bind_text(
               m_stmt, index, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT) ==
           SQLITE_OK;
}

bool Statement::BindTextStatic(int index, std::string_view value)
{
    return sqlite3_bind_text(
               m_stmt, index, value.data(), static_cast<int>(value.size()), SQLITE_STATIC) == SQLITE_OK;
}

EStepResult Statement::Step()
{
    int const rc = sqlite3_step(m_stmt);
    if (rc == SQLITE_ROW)
    {
        return EStepResult::Row;
    }
    if (rc == SQLITE_DONE)
    {
        return EStepResult::Done;
    }
    return EStepResult::Error;
}

bool Statement::Reset()
{
    return sqlite3_reset(m_stmt) == SQLITE_OK;
}

std::size_t Statement::GetColumnCount() const
{
    return static_cast<std::size_t>(sqlite3_column_count(m_stmt));
}

std::int64_t Statement::GetColumnInt64(int col) const
{
    return static_cast<std::int64_t>(sqlite3_column_int64(m_stmt, col));
}

char const* Statement::GetColumnText(int col) const
{
    return reinterpret_cast<char const*>(sqlite3_column_text(m_stmt, col));
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite
