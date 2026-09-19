/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Adapter.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief ABI Safe/Unsafe data adapters
///

#include <cstring>

#include "Fluxion/API/LogsPlugin/Private/ABI/Adapter.hpp"

#include "Graphite/Logger.hpp"
DEFINE_LOG_SCOPE(Fluxion::API::LogsPlugin::Private::ABI::Adapter);
USE_LOG_SCOPE(Fluxion::API::LogsPlugin::Private::ABI::Adapter);

namespace Fluxion::API::LogsPlugin::Private::ABI::Adapter {

std::string_view ToNative(Safe::StringView const string) noexcept
{
    return {string.data, string.size};
}

Unsafe::OnEnableData ToNative(Safe::OnEnableData const& data)
{
    return Unsafe::OnEnableData{
        .plugin_home_path = std::filesystem::path{ToNative(data.plugin_home_path)}};
}

Unsafe::OnDisableData ToNative(Safe::OnDisableData const&)
{
    return {};
}

Unsafe::Condition ToNative(Safe::Condition const& condition)
{
    Unsafe::Condition result{
        .column_index = condition.column_index,
        .data = std::string{condition.data.data, condition.data.size}};

    result[Safe::EConditionFlag::IsRegex] =
        (condition.flags & static_cast<std::uint8_t>(Safe::EConditionFlag::IsRegex)) != 0;

    result[Safe::EConditionFlag::IsEquals] =
        (condition.flags & static_cast<std::uint8_t>(Safe::EConditionFlag::IsEquals)) != 0;

    result[Safe::EConditionFlag::IsCaseSensitive] =
        (condition.flags & static_cast<std::uint8_t>(Safe::EConditionFlag::IsCaseSensitive)) != 0;

    return result;
}

Unsafe::Filter ToNative(Safe::Filter const& filter)
{
    Unsafe::Filter result{
        .id = ToNative(filter.id),
        .conditions{},
        .colors{},
        .priority = filter.priority,
        .highlight_only = filter.highlight_only != 0};

    result.conditions.reserve(filter.conditions.size);

    for (auto const& condition : ToNative(filter.conditions))
    {
        result.conditions.emplace_back(ToNative(condition));
    }

    return result;
}

Unsafe::ColumnDetails ToNative(Safe::ColumnDetails const& column)
{
    return Unsafe::ColumnDetails{
        .id = ToNative(column.id),
        .display_name = std::string{column.display_name.data, column.display_name.size}};
}

Unsafe::LogRowMetadata ToNative(Safe::LogRowMetadata const& metadata)
{
    return Unsafe::LogRowMetadata{
        .filter_id = ToNative(metadata.filter_id), .highlight_id = ToNative(metadata.highlight_id)};
}

Graphite::Common::Utility::UniqueID ToNative(Safe::UniqueID const& id) noexcept
{
    std::array<unsigned char, 16> data{};
    std::copy(std::begin(id.data), std::end(id.data), data.begin());
    LOG_DEBUG("::ToNative(UniqueID): {}", Graphite::Common::Utility::UniqueID::FromData(data));

    return Graphite::Common::Utility::UniqueID::FromData(data);
}

std::span<Safe::Condition const> ToNative(Safe::ConditionsSpan const conditions) noexcept
{
    return {conditions.data, conditions.size};
}

std::span<Safe::Filter const> ToNative(Safe::FiltersSpan const filters) noexcept
{
    return {filters.data, filters.size};
}

std::span<Safe::ColumnDetails const> ToNative(Safe::ColumnsDetailsSpan const columns) noexcept
{
    return {columns.data, columns.size};
}

std::span<Safe::Range const> ToNative(Safe::RangesSpan const ranges) noexcept
{
    return {ranges.data, ranges.size};
}

Safe::StringView ToSafe(std::string_view const string) noexcept
{
    return {.data = string.data(), .size = string.size()};
}

Safe::OnEnableData ToSafe(Unsafe::OnEnableData const& data)
{
    auto const path = data.plugin_home_path.u8string();

    return Safe::OnEnableData{
        .plugin_home_path = ToSafe({reinterpret_cast<char const*>(path.data()), path.size()})};
}

Safe::OnDisableData ToSafe(Unsafe::OnDisableData const&)
{
    return {};
}

Safe::Condition ToSafe(Unsafe::Condition const& condition)
{
    std::uint8_t flags{};

    if (condition[Safe::EConditionFlag::IsRegex])
        flags |= static_cast<std::uint8_t>(Safe::EConditionFlag::IsRegex);

    if (condition[Safe::EConditionFlag::IsEquals])
        flags |= static_cast<std::uint8_t>(Safe::EConditionFlag::IsEquals);

    if (condition[Safe::EConditionFlag::IsCaseSensitive])
        flags |= static_cast<std::uint8_t>(Safe::EConditionFlag::IsCaseSensitive);

    return Safe::Condition{
        .column_index = condition.column_index, .data = ToSafe(condition.data), .flags = flags};
}

Safe::ColumnDetails ToSafe(Unsafe::ColumnDetails const& column)
{
    return Safe::ColumnDetails{.id = ToSafe(column.id), .display_name = ToSafe(column.display_name)};
}

Safe::LogRowMetadata ToSafe(Unsafe::LogRowMetadata const& metadata)
{
    return Safe::LogRowMetadata{
        .filter_id = ToSafe(metadata.filter_id), .highlight_id = ToSafe(metadata.highlight_id)};
}

Safe::UniqueID ToSafe(Graphite::Common::Utility::UniqueID const& id) noexcept
{
    Safe::UniqueID result{};

    auto const& data = id.Data();

    std::copy(data.begin(), data.end(), std::begin(result.data));

    return result;
}

} // namespace Fluxion::API::LogsPlugin::Private::ABI::Adapter
