/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief General data.
///

#pragma once

#include <format>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "Core/Common/UniqueID.hpp"

#include "imgui/imgui.h"

namespace Fluxion::API::Data {

namespace Internal {

template <typename Derived, typename Enum>
    requires std::is_enum_v<Enum> && std::is_unsigned_v<std::underlying_type_t<Enum>>
class TWithFlags
{
public:
    using Storage = std::underlying_type_t<Enum>;

    ///
    /// @brief Set a flag
    /// @param flag to be set
    /// @return Ref to Derived object
    ///
    inline Derived& operator+=(Enum const flag) noexcept
    {
        flags |= static_cast<Storage>(flag);
        return static_cast<Derived&>(*this);
    }

    ///
    /// @brief Clear a flag
    /// @param flag to be cleared
    /// @return Ref to Derived object
    ///
    inline Derived& operator-=(Enum const flag) noexcept
    {
        flags &= ~static_cast<Storage>(flag);
        return static_cast<Derived&>(*this);
    }

    ///
    /// @brief Reset all flags
    ///
    inline void ClearFlags() noexcept { flags = 0; }

    ///
    /// @brief Check if a flag is set
    /// @param flag to be checked
    /// @return wether the flag is set or not
    ///
    [[nodiscard]]
    inline bool operator[](Enum const flag) const noexcept
    {
        return (flags & static_cast<Storage>(flag)) != 0;
    }

protected:
    Storage flags{};
};

} // namespace Internal

namespace Plugin {

struct OnEnableData
{
};

struct OnDisableData
{
};

}; // namespace Plugin

using LogsTableHeader = std::vector<std::string>;
using LogRow = std::vector<std::string>;
using LogsChunk = std::vector<LogRow>;

enum class EFilterComponentFlag : std::uint8_t
{
    None = 0,
    IsRegex = 1 << 0, // 00000001
    IsEquals = 1 << 1, // 00000010
    IsIgnoreCase = 1 << 2 // 00000100
};

struct FilterComponent : public Internal::TWithFlags<FilterComponent, EFilterComponentFlag>
{
    Graphite::Core::Common::UniqueID uid{};
    Graphite::Core::Common::UniqueID over_field_id{};
    std::string data{};

    friend std::ostream& operator<<(std::ostream& os, const FilterComponent& v);
};

struct FilterColors
{
    ImVec4 foreground{};
    ImVec4 background{};
};

enum class EFilterFlag : std::uint8_t
{
    None = 0,
    IsActive = 1 << 0, // 00000001
    IsHighlightOnly = 1 << 1, // 00000010
    IsCollapsed = 1 << 2, // 00000100
};

struct Filter : public Internal::TWithFlags<Filter, EFilterFlag>
{
    Graphite::Core::Common::UniqueID uid{};
    std::string name{};
    std::vector<Graphite::Core::Common::UniqueID> component_uids{};
    FilterColors colors{};
    std::uint8_t priority{};

    friend std::ostream& operator<<(std::ostream& os, const Filter& v);
};

enum class EFilterTabFlag : std::uint8_t
{
    None = 0,
    IsActive = 1 << 0 // 00000001
};

struct FilterTab : public Internal::TWithFlags<FilterTab, EFilterFlag>
{
    Graphite::Core::Common::UniqueID uid{};
    std::string name{};
    std::vector<Graphite::Core::Common::UniqueID> filter_ids{};

    friend std::ostream& operator<<(std::ostream& os, const FilterTab& v);
};

struct Filters
{
    std::vector<FilterTab> tabs{};
    std::vector<Filter> filters{};
    std::vector<FilterComponent> components{};
};

} // namespace Fluxion::API::Data
