/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Bridge.hpp
/// @author Alexandru Delegeanu
/// @version 0.15
/// @brief General data.
///

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include "Fluxion/API/Data/Common.hpp"
#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

#include "Fluxion/API/Data/Common.hpp"

namespace Fluxion::API::LogsPlugin::Bridge {

namespace ABI {

struct StringView
{
    const char* data{nullptr};
    std::size_t size{0};

    inline operator std::string_view() const { return {data, size}; }
    inline StringView() = default;
    inline StringView(std::string_view const sv) : data(sv.data()), size(sv.size()) {}
    inline StringView(const char* str)
        : StringView(str != nullptr ? std::string_view{str} : std::string_view{})
    {
    }
    inline StringView(std::string const& str) : data(str.data()), size(str.size()) {}
};

template <typename T>
struct Span
{
    const T* data{nullptr};
    std::size_t size{0};

    inline operator std::span<const T>() const { return {data, size}; }
    inline Span() = default;
    inline Span(std::span<const T> const s) : data(s.data()), size(s.size()) {}
    inline Span(const std::vector<T>& v) : data(v.data()), size(v.size()) {}
};

} // namespace ABI

struct OnEnableData
{
    ABI::StringView plugin_home_path{};

    inline std::filesystem::path GetPath() const
    {
        return std::filesystem::path(std::string_view(plugin_home_path));
    }
};

struct OnDisableData
{
};

// clang-format off
enum class EConditionFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsRegex         = 1 << 0, // 00000001
    IsEquals        = 1 << 1, // 00000010
    IsCaseSensitive = 1 << 2, // 00000100
};
// clang-format on

struct Condition : public Graphite::Common::Utility::TWithFlags<Condition, EConditionFlag>
{
    std::size_t column_index{};
    ABI::StringView data{};
};

struct Filter
{
    Graphite::Common::Utility::UniqueID id{};
    ABI::Span<Condition> conditions{};
    Fluxion::API::Data::Common::Highlight colors{};
    std::uint8_t priority{};
    bool highlight_only{false};
};

struct ColumnDetails
{
    Graphite::Common::Utility::UniqueID id{};
    ABI::StringView display_name{};
};

struct Range
{
    std::size_t begin{};
    std::size_t end{};
};

struct LogRowMetadata
{
    Graphite::Common::Utility::UniqueID filter_id{Graphite::Common::Utility::UniqueID::GetDefault()};
    Graphite::Common::Utility::UniqueID highlight_id{Graphite::Common::Utility::UniqueID::GetDefault()};
};

class ILogsWriter
{
public:
    virtual ~ILogsWriter() = default;

    virtual void WriteData(std::size_t const log_index, ABI::Span<ABI::StringView> const columns) = 0;

    virtual void WriteMetadata(
        std::size_t const log_index,
        Graphite::Common::Utility::UniqueID const filter_id,
        Graphite::Common::Utility::UniqueID const highlight_id) = 0;
};

enum class ELogsOperationUnit : std::uint8_t
{
    None = 0,
    Logs = 1,
    Bytes = 2
};

} // namespace Fluxion::API::LogsPlugin::Bridge
