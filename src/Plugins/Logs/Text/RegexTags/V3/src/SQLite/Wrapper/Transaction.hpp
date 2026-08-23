/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Transaction.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief RAII manager for SQLite transactions
///

#pragma once

#include "DatabaseRef.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

class Transaction
{
public: // Lifecycle
    explicit Transaction(DatabaseRef db);
    ~Transaction();

    Transaction(Transaction const&) = delete;
    Transaction& operator=(Transaction const&) = delete;

    Transaction(Transaction&& other) noexcept;
    Transaction& operator=(Transaction&& other) noexcept;

public: // API
    bool Commit();
    void Rollback();

    [[nodiscard]] bool IsActive() const;

private:
    DatabaseRef m_db;
    bool m_active{false};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
