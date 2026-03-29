/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file UniqueID.cpp
/// @author Alexandru Delegeanu
/// @version 1.6
/// @brief Implementation of @see UniqueID.hpp.
///

#include <iostream>
#include <random>
#include <utility>

#include "Graphite/Common/Utility/UniqueID.hpp"

namespace Graphite::Common::Utility {

UniqueID::UniqueID(UniqueID&& other) noexcept : m_data{std::move(other.m_data)}
{
    other.m_data = s_default.m_data;
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
    // Use thread_local so we don't recreate the generator
    // and don't need a mutex for thread safety.
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<std::uint32_t> dist(0, 0xFFFFFFFF);

    // Generate 4 chunks of 32-bit ints (16 bytes total)
    uint32_t* ptr = reinterpret_cast<uint32_t*>(out.m_data.data());
    ptr[0] = dist(generator);
    ptr[1] = dist(generator);
    ptr[2] = dist(generator);
    ptr[3] = dist(generator);

    // Set UUID v4 bits
    out.m_data[6] = (out.m_data[6] & 0x0F) | 0x40;
    out.m_data[8] = (out.m_data[8] & 0x3F) | 0x80;

    return out;
}

UniqueID UniqueID::Default()
{
    UniqueID out{};
    out.m_data.fill('0');
    return out;
}

[[nodiscard]] std::string UniqueID::ToString() const
{
    if (!initialized())
    {
        return "00000000-0000-0000-0000-000000000000";
    }

    std::string result{};
    result.resize(36);

    static constexpr char hex[] = "0123456789abcdef";
    char* ptr = result.data();

    for (std::size_t i = 0; i < m_data.size(); ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            *ptr++ = '-';
        }

        unsigned char byte = m_data[i];
        *ptr++ = hex[(byte >> 4) & 0xF];
        *ptr++ = hex[byte & 0xF];
    }

    return result;
}

void UniqueID::Dump(char dst[37]) const
{
    static constexpr char hex_chars[] = "0123456789abcdef";

    char* out = dst;
    for (std::size_t i = 0; i < m_data.size(); ++i)
    {
        // Add dashes at UUID standard positions
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            *out++ = '-';
        }

        unsigned char byte = m_data[i];
        *out++ = hex_chars[(byte >> 4) & 0x0F]; // High nibble
        *out++ = hex_chars[byte & 0x0F]; // Low nibble
    }
    *out = '\0';
}

std::ostream& operator<<(std::ostream& os, UniqueID const& rhs)
{
    return os << rhs.ToString();
}

std::string operator+(std::string_view const lhs, UniqueID const& rhs)
{
    static constexpr char hex[] = "0123456789abcdef";

    std::string result;
    result.reserve(lhs.size() + 36);
    result.append(lhs);

    for (std::size_t i = 0; i < rhs.m_data.size(); ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            result.push_back('-');
        }

        unsigned char byte = rhs.m_data[i];
        result.push_back(hex[(byte >> 4) & 0xF]);
        result.push_back(hex[byte & 0xF]);
    }

    return result;
}

std::string operator+(UniqueID const& lhs, std::string_view const rhs)
{
    return operator+(rhs, lhs);
}

bool UniqueID::initialized() const
{
    return s_default != *this;
}

std::size_t UniqueID::Hash::operator()(UniqueID const& id) const
{
    // A robust, fast hash for 16 bytes of data.
    // We treat the 16 bytes as two 64-bit halves.
    uint64_t const* words = reinterpret_cast<const uint64_t*>(id.m_data.data());

    // Simple but effective combination (similar to boost::hash_combine)
    std::size_t h1 = std::hash<uint64_t>{}(words[0]);
    std::size_t h2 = std::hash<uint64_t>{}(words[1]);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
}

const UniqueID UniqueID::s_default = UniqueID::Default();

} // namespace Graphite::Common::Utility
