/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Adapter.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief ABI Safe/Unsafe data adapters
///

#pragma once

#include <span>
#include <string_view>

#include "Fluxion/API/LogsPlugin/Private/ABI/Safe.hpp"
#include "Fluxion/API/LogsPlugin/Private/ABI/Unsafe.hpp"

namespace Fluxion::API::LogsPlugin::Private::ABI::Adapter {

// -- ToNative

[[nodiscard]]
std::string_view ToNative(Safe::StringView const string) noexcept;

[[nodiscard]]
Unsafe::OnEnableData ToNative(Safe::OnEnableData const& data);

[[nodiscard]]
Unsafe::OnDisableData ToNative(Safe::OnDisableData const& data);

[[nodiscard]]
Unsafe::Condition ToNative(Safe::Condition const& condition);

[[nodiscard]]
Unsafe::Filter ToNative(Safe::Filter const& filter);

[[nodiscard]]
Unsafe::ColumnDetails ToNative(Safe::ColumnDetails const& column);

[[nodiscard]]
Unsafe::LogRowMetadata ToNative(Safe::LogRowMetadata const& metadata);

[[nodiscard]]
Graphite::Common::Utility::UniqueID ToNative(Safe::UniqueID const& id) noexcept;

[[nodiscard]]
std::span<Safe::Condition const> ToNative(Safe::ConditionsSpan const conditions) noexcept;

[[nodiscard]]
std::span<Safe::Filter const> ToNative(Safe::FiltersSpan const filters) noexcept;

[[nodiscard]]
std::span<Safe::ColumnDetails const> ToNative(Safe::ColumnsDetailsSpan const columns) noexcept;

[[nodiscard]]
std::span<Safe::Range const> ToNative(Safe::RangesSpan const ranges) noexcept;

// --- ToSafe

[[nodiscard]]
Safe::StringView ToSafe(std::string_view const string) noexcept;

[[nodiscard]]
Safe::OnEnableData ToSafe(Unsafe::OnEnableData const& data);

[[nodiscard]]
Safe::OnDisableData ToSafe(Unsafe::OnDisableData const& data);

[[nodiscard]]
Safe::Condition ToSafe(Unsafe::Condition const& condition);

[[nodiscard]]
Safe::ColumnDetails ToSafe(Unsafe::ColumnDetails const& column);

[[nodiscard]]
Safe::LogRowMetadata ToSafe(Unsafe::LogRowMetadata const& metadata);

[[nodiscard]]
Safe::UniqueID ToSafe(Graphite::Common::Utility::UniqueID const& id) noexcept;

} // namespace Fluxion::API::LogsPlugin::Private::ABI::Adapter
