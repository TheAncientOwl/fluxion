/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DatabaseRef.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Non-owning safe handle wrapper for sqlite3 connection
///

#pragma once

#include "Statement.hpp"

#include <functional>
#include <string>
#include <string_view>

struct sqlite3;

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite {

class DatabaseRef
{
public:
    using ProgressCallback = std::function<int()>;

public: // Lifecycle
    explicit DatabaseRef(sqlite3* db);

    DatabaseRef(DatabaseRef const&) = default;
    DatabaseRef& operator=(DatabaseRef const&) = default;

    DatabaseRef(DatabaseRef&&) noexcept = default;
    DatabaseRef& operator=(DatabaseRef&&) noexcept = default;

public: // API
    sqlite3* GetRaw() = delete;

    [[nodiscard]] bool Execute(std::string_view sql, std::string* out_err = nullptr) const;
    [[nodiscard]] bool ExecuteTransaction(std::function<bool()> const& transaction_func) const;

    [[nodiscard]] Statement Prepare(std::string_view sql) const;

    [[nodiscard]] char const* GetLastErrorMessage() const;

    /**
     * @brief Registers a progress handler callback triggered every num_opcodes instructions.
     * @note The caller must ensure 'callback' remains valid until ClearProgressHandler is called.
     */
    void SetProgressHandler(int num_opcodes, ProgressCallback const& callback) const;
    void ClearProgressHandler() const;

private:
    sqlite3* m_db{nullptr};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite
