/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Transaction.cpp
/// @author Alexandru Delegeanu
/// @version 4.0
/// @brief Implementation of @see Transaction.hpp
///

#include "Transaction.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V4::SQLite {

Transaction::Transaction(DatabaseRef db) : m_db{db}, m_active{m_db.Execute("BEGIN TRANSACTION;")}
{
}

Transaction::~Transaction()
{
    if (m_active)
    {
        std::ignore = m_db.Execute("ROLLBACK;");
    }
}

Transaction::Transaction(Transaction&& other) noexcept : m_db{other.m_db}, m_active{other.m_active}
{
    other.m_active = false;
}

Transaction& Transaction::operator=(Transaction&& other) noexcept
{
    if (this != &other)
    {
        if (m_active)
        {
            std::ignore = m_db.Execute("ROLLBACK;");
        }
        m_db = other.m_db;
        m_active = other.m_active;
        other.m_active = false;
    }
    return *this;
}

bool Transaction::Commit()
{
    if (m_active && m_db.Execute("COMMIT;"))
    {
        m_active = false;
        return true;
    }
    return false;
}

void Transaction::Rollback()
{
    if (m_active)
    {
        std::ignore = m_db.Execute("ROLLBACK;");
        m_active = false;
    }
}

bool Transaction::IsActive() const
{
    return m_active;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V4::SQLite
