/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Safe.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief ABI Safe data structures
///

#pragma once

#include <cstddef>
#include <cstdint>

namespace Fluxion::API::LogsPlugin::Private::ABI::Safe {

// clang-format off
enum class EConditionFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsRegex         = 1 << 0, // 00000001
    IsEquals        = 1 << 1, // 00000010
    IsCaseSensitive = 1 << 2, // 00000100
};
// clang-format on

enum class ELogsOperationUnit : std::uint8_t
{
    None = 0,
    Logs = 1,
    Bytes = 2
};

struct StringView
{
    char const* data{};
    std::size_t size{};
};

struct OnEnableData
{
    StringView plugin_home_path{};
};

struct OnDisableData
{
};

struct Condition
{
    std::size_t column_index{};
    StringView data{};
    std::uint8_t flags{};
};

struct UniqueID
{
    std::uint8_t data[16]{};
};

struct ConditionsSpan
{
    Condition const* data{};
    std::size_t size{};
};

struct Filter
{
    UniqueID id{};
    ConditionsSpan conditions{};
    std::uint8_t priority{};
    std::uint8_t highlight_only{};
};

struct FiltersSpan
{
    Filter const* data{};
    std::size_t size{};
};

struct ColumnDetails
{
    UniqueID id{};
    StringView display_name;
};

struct ColumnsDetailsSpan
{
    ColumnDetails const* data{};
    std::size_t size{};
};

struct Range
{
    std::size_t begin{};
    std::size_t end{};
};

struct RangesSpan
{
    Range const* data{};
    std::size_t size{};
};

struct LogRowMetadata
{
    UniqueID filter_id;
    UniqueID highlight_id;
};

struct StringViewsSpan
{
    StringView const* data{};
    std::size_t size{};
};

using LogRowData = StringViewsSpan;

using WriteLogRowDataFn = void (*)(void* user_data, std::size_t const index, LogRowData const* data);
using WriteLogRowMetadataFn =
    void (*)(void* user_data, std::size_t const index, LogRowMetadata const* metadata);
struct LogRowWriter
{
    void* user_data{};
    WriteLogRowDataFn write_data{};
    WriteLogRowMetadataFn write_metadata{};
};

} // namespace Fluxion::API::LogsPlugin::Private::ABI::Safe
