/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file QueryExecutor.hpp
/// @author Alexandru Delegeanu
/// @version 3.0
/// @brief RAII SQLite query execution and transaction wrapper with lifecycle management.
///

#pragma once

#include <functional>
#include <string_view>
#include "OpenCloseManager.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite {

class QueryExecutor : public OpenCloseManager
{
public:
    using ProgressCallback = std::function<int()>;

public:
    explicit QueryExecutor(std::filesystem::path const& db_path) noexcept;

public:
    [[nodiscard]] bool Execute(std::string_view const sql_query) const;
    [[nodiscard]] bool ExecuteTransaction(std::function<bool()> const& transaction_func) const;

    void AddProgressHandler(int opcode_intervals, ProgressCallback callback);
    void RemoveProgressHandler();
    [[nodiscard]] std::size_t GetFilteredLogsCount() const;

private:
    static int SQLiteProgressCallbackTrampoline(void* arg);

private:
    ProgressCallback m_progress_callback{nullptr};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V3::SQLite
