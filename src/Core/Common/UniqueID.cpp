/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file UniqueID.cpp
/// @author Alexandru Delegeanu
/// @version 1.2
/// @brief Implementation of @see UniqueID.hpp.
///

#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <ranges>
#include <sstream>
#include <utility>

#include "UniqueID.hpp"

namespace Graphite::Core::Common {

UniqueID::UniqueID(UniqueID&& other) noexcept : m_data{std::move(other.m_data)}
{
}

UniqueID& UniqueID::operator=(UniqueID&& rhs) noexcept
{
    m_data = std::move(rhs.m_data);
    rhs.m_data = s_default.m_data;
    return *this;
}

bool UniqueID::operator<(UniqueID const& rhs) const
{
    return m_data < rhs.m_data;
}

bool UniqueID::operator==(UniqueID const& rhs) const
{
    return m_data == rhs.m_data;
}

UniqueID UniqueID::Generate()
{
    UniqueID out{};
    auto& uid = out.m_data;

    std::random_device random_device{};
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<std::uint16_t> distribution(0, 255);

    for (auto& byte : uid)
    {
        byte = distribution(generator);
    }

    // Set version to 4 (0b0100xxxx)
    uid[6] = (uid[6] & 0x0F) | 0x40;

    // Set variant to 2 (0b10xxxxxx)
    uid[8] = (uid[8] & 0x3F) | 0x80;

    return out;
}

UniqueID UniqueID::Default()
{
    UniqueID out{};
    out.m_data.fill('0');
    return out;
}

std::string UniqueID::toString() const
{
    if (!initialized())
    {
        return "";
    }

    std::ostringstream oss{};

    for (auto const index : std::ranges::iota_view{0u, m_data.size()})
    {
        if (index == 4 || index == 6 || index == 8 || index == 10)
        {
            oss << "-";
        }
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(m_data[index]);
    }

    return oss.str();
}

std::ostream& operator<<(std::ostream& os, UniqueID const& rhs)
{
    return os << rhs.toString();
}

bool UniqueID::initialized() const
{
    return s_default != *this;
}

std::size_t UniqueID::Hash::operator()(UniqueID const& id) const
{
    auto const data_hash{std::hash<std::string>{}(id.toString())};
    return data_hash;
}

const UniqueID UniqueID::s_default = UniqueID::Default();

} // namespace Graphite::Core::Common
