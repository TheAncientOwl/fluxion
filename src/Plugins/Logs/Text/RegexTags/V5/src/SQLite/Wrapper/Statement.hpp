/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Statement.hpp
/// @author Alexandru Delegeanu
/// @version 5.0
/// @brief RAII wrapper for sqlite3_stmt operations
///

#pragma once

#include <cstdint>
#include <string_view>

struct sqlite3_stmt;

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite {

enum class EStepResult
{
    Row,
    Done,
    Error
};

class Statement
{
public: // Lifecycle
    explicit Statement(sqlite3_stmt* stmt = nullptr);
    ~Statement();

    Statement(Statement const&) = delete;
    Statement& operator=(Statement const&) = delete;

    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

public: // API
    [[nodiscard]] bool IsValid() const;

    bool BindInt64(int index, std::int64_t value);
    bool BindText(int index, std::string_view value);

    EStepResult Step();
    bool Reset();

    [[nodiscard]] std::size_t GetColumnCount() const;
    [[nodiscard]] std::int64_t GetColumnInt64(int col) const;
    [[nodiscard]] char const* GetColumnText(int col) const;

private:
    sqlite3_stmt* m_stmt{nullptr};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::SQLite
