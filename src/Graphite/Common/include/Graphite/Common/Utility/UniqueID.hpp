/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file UniqueID.hpp
/// @author Alexandru Delegeanu
/// @version 1.5
/// @brief UniqueID abstraction.
///

#pragma once

#include <array>
#include <format>
#include <ostream>

namespace Graphite::Common::Utility {

class UniqueID
{
public: // lifecycle
    UniqueID() = default;
    ~UniqueID() = default;
    UniqueID(UniqueID const&) = default;
    UniqueID& operator=(UniqueID const&) = default;
    UniqueID(UniqueID&&) noexcept;
    UniqueID& operator=(UniqueID&&) noexcept;

public: // public API
    ///
    /// @brief Self explanatory
    ///
    static UniqueID Generate();

    ///
    /// @brief Self explanatory
    ///
    static UniqueID Default();

    ///
    /// @brief Self explanatory
    /// @return ID in format 8f674453-a065-4138-969d-10b6b83b94cc
    /// @return ID in format OOOOOOOO-OOOO-OOOO-OOOO-OOOOOOOOOOOO
    ///
    [[nodiscard]] std::string ToString() const;

    ///
    /// @brief Dumps hex representation into char array, '\0' terminated.
    ///
    void Dump(char dst[37]) const;

    static inline constexpr std::size_t GetMinDumpSize() { return 37; }

    ///
    /// @return true if object was created via UniqueID::generate, false otherwise
    ///
    bool initialized() const;

public: // hashing
    struct Hash
    {
        std::size_t operator()(UniqueID const&) const;
    };
    friend std::size_t Hash::operator()(UniqueID const&) const;

public: // operators
    bool operator<(UniqueID const&) const;
    bool operator==(UniqueID const&) const;
    friend std::ostream& operator<<(std::ostream&, UniqueID const&);
    friend struct std::formatter<UniqueID>;
    friend std::string operator+(std::string_view const lhs, UniqueID const& rhs);
    friend std::string operator+(UniqueID const& lhs, std::string_view const rhs);

private: // fields
    static const UniqueID s_default;
    std::array<unsigned char, 16> m_data{};
};

std::string operator+(std::string_view const lhs, UniqueID const& rhs);
std::string operator+(UniqueID const& lhs, std::string_view const rhs);

} // namespace Graphite::Common::Utility

template <>
struct std::formatter<Graphite::Common::Utility::UniqueID>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(Graphite::Common::Utility::UniqueID const& p, std::format_context& ctx) const
    {
        static constexpr char hex[] = "0123456789abcdef";

        auto out = ctx.out();

        for (std::size_t i = 0; i < p.m_data.size(); ++i)
        {
            if (i == 4 || i == 6 || i == 8 || i == 10)
            {
                *out++ = '-';
            }

            unsigned char byte = p.m_data[i];
            *out++ = hex[(byte >> 4) & 0xF];
            *out++ = hex[byte & 0xF];
        }

        return out;
    }
};
