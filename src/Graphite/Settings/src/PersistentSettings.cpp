/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file PersistentSettings.cpp
/// @author Alexandru Delegeanu
/// @version 1.17
/// @brief Settings management using JSON
///

#include "Graphite/Settings/PersistentSettings.hpp"

#include <fstream>
#include <vector>

namespace Graphite::Settings {

PersistentSettings::PersistentSettings(std::filesystem::path home, std::string file_name)
    : m_file_path(home / (file_name + ".json"))
{
    Load();
}

void PersistentSettings::Load()
{
    if (!std::filesystem::exists(m_file_path))
    {
        m_data = json::object();
        return;
    }

    try
    {
        std::ifstream file(m_file_path);
        if (file.is_open())
        {
            file >> m_data;
            file.close();
        }
    }
    catch (const std::exception& e)
    {
        // If JSON parsing fails, start fresh
        m_data = json::object();
    }
}

void PersistentSettings::Save()
{
    try
    {
        // Create directory if it doesn't exist
        auto dir = m_file_path.parent_path();
        if (!std::filesystem::exists(dir))
        {
            std::filesystem::create_directories(dir);
        }

        std::ofstream file(m_file_path);
        if (file.is_open())
        {
            file << m_data.dump(4); // Pretty print with 4 spaces
            file.close();
        }
    }
    catch (const std::exception& e)
    {
        // Silently fail on write errors
    }
}

void PersistentSettings::erase(std::string_view key)
{
    if (m_data.contains(key))
    {
        m_data.erase(key);
    }
}

std::vector<std::string> PersistentSettings::GetKeys() const
{
    std::vector<std::string> keys;
    for (auto const& [key, _] : m_data.items())
    {
        keys.push_back(key);
    }
    return keys;
}

void PersistentSettings::SetJsonValue(std::string_view key, json const& value)
{
    m_data[key] = value;
}

std::optional<json> PersistentSettings::GetJsonValue(std::string_view key) const
{
    if (m_data.contains(key))
    {
        return m_data[key];
    }
    return std::nullopt;
}

// Macros to reduce boilerplate
#define DEFINE_SET_SCALAR(Type)                                                 \
    template <>                                                                 \
    void PersistentSettings::set<Type>(std::string_view key, Type const& value) \
    {                                                                           \
        m_data[key] = value;                                                    \
    }

#define DEFINE_GET_SCALAR(Type, Condition)                                  \
    template <>                                                             \
    std::optional<Type> PersistentSettings::get<Type>(std::string_view key) \
    {                                                                       \
        if (m_data.contains(key) && m_data[key].Condition)                  \
        {                                                                   \
            return m_data[key].get<Type>();                                 \
        }                                                                   \
        return std::nullopt;                                                \
    }

#define DEFINE_SET_VECTOR(Type)                               \
    template <>                                               \
    void PersistentSettings::set<std::vector<Type>>(          \
        std::string_view key, std::vector<Type> const& value) \
    {                                                         \
        m_data[key] = value;                                  \
    }

#define DEFINE_GET_VECTOR(Type)                                                                       \
    template <>                                                                                       \
    std::optional<std::vector<Type>> PersistentSettings::get<std::vector<Type>>(std::string_view key) \
    {                                                                                                 \
        if (m_data.contains(key) && m_data[key].is_array())                                           \
        {                                                                                             \
            try                                                                                       \
            {                                                                                         \
                return m_data[key].get<std::vector<Type>>();                                          \
            }                                                                                         \
            catch (const std::exception&)                                                             \
            {                                                                                         \
                return std::nullopt;                                                                  \
            }                                                                                         \
        }                                                                                             \
        return std::nullopt;                                                                          \
    }

// Scalar types
DEFINE_SET_SCALAR(std::string)
DEFINE_GET_SCALAR(std::string, is_string())

DEFINE_SET_SCALAR(std::size_t)
DEFINE_GET_SCALAR(std::size_t, is_number_integer())

DEFINE_SET_SCALAR(int)
DEFINE_GET_SCALAR(int, is_number_integer())

DEFINE_SET_SCALAR(double)
DEFINE_GET_SCALAR(double, is_number())

DEFINE_SET_SCALAR(bool)
DEFINE_GET_SCALAR(bool, is_boolean())

// Vector types
DEFINE_SET_VECTOR(std::string)
DEFINE_GET_VECTOR(std::string)

DEFINE_SET_VECTOR(int)
DEFINE_GET_VECTOR(int)

DEFINE_SET_VECTOR(double)
DEFINE_GET_VECTOR(double)

DEFINE_SET_VECTOR(bool)
DEFINE_GET_VECTOR(bool)

#undef DEFINE_SET_SCALAR
#undef DEFINE_GET_SCALAR
#undef DEFINE_SET_VECTOR
#undef DEFINE_GET_VECTOR

} // namespace Graphite::Settings
