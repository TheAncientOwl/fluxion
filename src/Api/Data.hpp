/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief General data.
///

#pragma once

#include <memory>
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

    /**
     * @brief Proxy object to provide read/write access to individual flags.
     *
     * This proxy allows convenient manipulation of specific flags within the flags storage.
     * It supports conversion to bool to check if the flag is set, and assignment from bool
     * to set or clear the flag.
     */
    struct FlagProxy
    {
        Storage& flags;
        Enum flag;

        /**
         * @brief Conversion operator to check if the flag is set.
         * @return true if the flag is set, false otherwise.
         */
        operator bool() const noexcept { return (flags & static_cast<Storage>(flag)) != 0; }

        /**
         * @brief Assign a boolean value to set or clear the flag.
         * @param value true to set the flag, false to clear it.
         * @return Reference to this FlagProxy.
         */
        FlagProxy& operator=(bool value) noexcept
        {
            if (value)
                flags |= static_cast<Storage>(flag);
            else
                flags &= ~static_cast<Storage>(flag);
            return *this;
        }
    };

    /**
     * @brief Provides read/write access to a specific flag.
     * @param flag The flag to access.
     * @return A FlagProxy object for the specified flag.
     */
    inline FlagProxy operator[](Enum const flag) noexcept { return FlagProxy{flags, flag}; }

    /**
     * @brief Reset all flags to zero (clear all flags).
     */
    inline void ClearFlags() noexcept { flags = 0; }

    /**
     * @brief Check if a specific flag is set.
     * @param flag The flag to check.
     * @return true if the flag is set, false otherwise.
     */
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

// clang-format off
enum class EFilterComponentFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsRegex         = 1 << 0, // 00000001
    IsEquals        = 1 << 1, // 00000010
    IsCaseSensitive = 1 << 2, // 00000100
};
// clang-format on

struct FilterComponent : public Internal::TWithFlags<FilterComponent, EFilterComponentFlag>
{
    Graphite::Core::Common::UniqueID id{};
    Graphite::Core::Common::UniqueID over_field_id{};
    std::string data{};

    FilterComponent() = default;
    FilterComponent(FilterComponent const& other);
    FilterComponent& operator=(FilterComponent const& other);
    FilterComponent(FilterComponent&& other) noexcept = default;
    FilterComponent& operator=(FilterComponent&& other) noexcept = default;
    ~FilterComponent() = default;

    friend std::ostream& operator<<(std::ostream& os, const FilterComponent& v);
};

struct FilterColors
{
    ImVec4 foreground{};
    ImVec4 background{};
};

// clang-format off
enum class EFilterFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsActive        = 1 << 0, // 00000001
    IsHighlightOnly = 1 << 1, // 00000010
    IsCollapsed     = 1 << 2, // 00000100
};
// clang-format on

struct Filter : public Internal::TWithFlags<Filter, EFilterFlag>
{
    Graphite::Core::Common::UniqueID id{};
    std::string name{};
    std::vector<std::shared_ptr<FilterComponent>> components{};
    FilterColors colors{};
    std::uint8_t priority{};

    Filter() = default;
    Filter(Filter const& other);
    Filter& operator=(Filter const& other);
    Filter(Filter&& other) noexcept;
    Filter& operator=(Filter&& other) noexcept;
    ~Filter() = default;

    friend std::ostream& operator<<(std::ostream& os, const Filter& v);
};

// clang-format off
enum class EFiltersTabFlag : std::uint8_t
{
    None     = 0,     // 00000000
    IsActive = 1 << 0 // 00000001
};
// clang-format on

struct FiltersTab : public Internal::TWithFlags<FiltersTab, EFiltersTabFlag>
{
    Graphite::Core::Common::UniqueID id{};
    std::string name{};
    std::vector<std::shared_ptr<Filter>> filters{};

    FiltersTab() = default;
    FiltersTab(FiltersTab const& other);
    FiltersTab& operator=(FiltersTab const& other);
    FiltersTab(FiltersTab&& other) noexcept = default;
    FiltersTab& operator=(FiltersTab&& other) noexcept = default;
    ~FiltersTab() = default;

    friend std::ostream& operator<<(std::ostream& os, const FiltersTab& v);
};

using FiltersTabs = std::vector<std::shared_ptr<FiltersTab>>;

} // namespace Fluxion::API::Data
