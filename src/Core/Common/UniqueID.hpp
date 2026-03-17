/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file UniqueID.hpp
/// @author Alexandru Delegeanu
/// @version 1.2
/// @brief UniqueID abstraction.
///

#pragma once

#include <array>
#include <functional>
#include <ostream>
#include <unordered_map>
#include <unordered_set>

namespace Graphite::Core::Common {

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
    std::string toString() const;

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

public: // structures
    template <typename Value>
    using MapTo = std::unordered_map<UniqueID, Value, Hash>;

public: // operators
    bool operator<(UniqueID const&) const;
    bool operator==(UniqueID const&) const;
    friend std::ostream& operator<<(std::ostream&, UniqueID const&);
    friend std::string operator+(std::string_view const lhs, UniqueID const& rhs);
    friend std::string operator+(UniqueID const& lhs, std::string_view const rhs);

private: // fields
    static const UniqueID s_default;
    std::array<unsigned char, 16> m_data{};
};

std::string operator+(std::string_view const lhs, UniqueID const& rhs);
std::string operator+(UniqueID const& lhs, std::string_view const rhs);

} // namespace Graphite::Core::Common
