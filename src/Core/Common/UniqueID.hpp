/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file UniqueID.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
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
    static UniqueID generate();

    ///
    /// @brief Self explanatory
    /// @return UID in format 8f674453-a065-4138-969d-10b6b83b94cc
    /// @return UID in format OOOOOOOO-OOOO-OOOO-OOOO-OOOOOOOOOOOO
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

private: // fields
    std::array<unsigned char, 16> m_data{};
    bool m_initialized{false};
};

} // namespace Graphite::Core::Common
