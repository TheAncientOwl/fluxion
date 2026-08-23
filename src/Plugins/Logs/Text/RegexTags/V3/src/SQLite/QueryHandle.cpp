/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file QueryHandle.cpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief Implementation of @see QueryHandle.hpp
///

#include "QueryHandle.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

QueryHandle::QueryHandle(sqlite3_stmt* stmt) : m_stmt(stmt)
{
}

QueryHandle::~QueryHandle()
{
    Finalize();
}

QueryHandle::QueryHandle(QueryHandle&& other) noexcept : m_stmt(other.m_stmt)
{
    other.m_stmt = nullptr;
}

QueryHandle& QueryHandle::operator=(QueryHandle&& other) noexcept
{
    if (this != &other)
    {
        Finalize();
        m_stmt = other.m_stmt;
        other.m_stmt = nullptr;
    }
    return *this;
}

QueryHandle::operator bool() const
{
    return m_stmt != nullptr;
}

sqlite3_stmt* QueryHandle::Get() const
{
    return m_stmt;
}

void QueryHandle::Finalize()
{
    if (m_stmt)
    {
        sqlite3_finalize(m_stmt);
        m_stmt = nullptr;
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
