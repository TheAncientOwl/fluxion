/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TWithFlags.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Utility class to reduce the size of an object when multiple flags (0/1) are used.
/// As in - use one single value, its bits representing flag states.
///

#pragma once

#include <type_traits>

namespace Graphite::Common::Utility {

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
    [[nodiscard]] inline bool operator[](Enum const flag) const noexcept
    {
        return (flags & static_cast<Storage>(flag)) != 0;
    }

protected:
    Storage flags{};
};

} // namespace Graphite::Common::Utility
