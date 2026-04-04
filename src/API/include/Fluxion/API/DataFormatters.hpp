/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DataFormatters.hpp
/// @author Alexandru Delegeanu
/// @version 0.7
/// @brief IO related utilities.
///

#pragma once

#include <format>
#include <memory>
#include <string>
#include <vector>

#include "imgui.h"

#include "Fluxion/API/Data.hpp"

// ==========================================================================
// 1. HEADER SHELLS (Declarations)
// ==========================================================================
// We define the structs in the std namespace so the compiler knows they exist,
// but we leave the format() function body for the end of the file.

namespace std {
template <typename T>
struct formatter<std::vector<T>> : std::formatter<std::string_view>
{
    auto format(const std::vector<T>& vec, format_context& ctx) const
    {
        std::string s = "[";
        for (size_t i = 0; i < vec.size(); ++i)
        {
            s += std::format("{}", vec[i]);
            if (i < vec.size() - 1)
                s += ", ";
        }
        s += "]";
        return std::formatter<std::string_view>::format(s, ctx);
    }
};

template <typename T>
struct formatter<std::shared_ptr<T>> : std::formatter<std::string_view>
{
    auto format(const std::shared_ptr<T>& ptr, format_context& ctx) const
    {
        if (!ptr)
            return std::formatter<std::string_view>::format("nullptr", ctx);
        return std::formatter<std::string_view>::format(std::format("{}", *ptr), ctx);
    }
};
} // namespace std

// ==========================================================================
// 2. CONCRETE TYPES (The "Meat")
// ==========================================================================

template <>
struct std::formatter<ImVec4> : std::formatter<std::string_view>
{
    auto format(const ImVec4& v, format_context& ctx) const
    {
        return std::formatter<std::string_view>::format(
            std::format("({:.2f}, {:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z, v.w), ctx);
    }
};

template <>
struct std::formatter<Fluxion::API::Data::FilterColors> : std::formatter<std::string_view>
{
    auto format(const Fluxion::API::Data::FilterColors& c, format_context& ctx) const
    {
        return std::formatter<std::string_view>::format(
            std::format("FG:{} BG:{}", c.foreground, c.background), ctx);
    }
};

template <>
struct std::formatter<Fluxion::API::Data::FilterComponent> : std::formatter<std::string_view>
{
    auto format(const Fluxion::API::Data::FilterComponent& fc, format_context& ctx) const
    {
        return std::formatter<std::string_view>::format(
            std::format("Component(ID: {}, Field: {}, Data: '{}')", fc.id, fc.over_field_id, fc.data),
            ctx);
    }
};

template <>
struct std::formatter<Fluxion::API::Data::Filter> : std::formatter<std::string_view>
{
    auto format(const Fluxion::API::Data::Filter& f, format_context& ctx) const
    {
        const auto& components = f.components.GetBack();
        // Now 'std::formatter<vector<shared_ptr<FilterComponent>>>' is "defined" (as a shell)
        // so this line won't trigger an "undefined template" error.
        return std::formatter<std::string_view>::format(
            std::format(
                "Filter(Name: '{}', Active: {}, Components: {})",
                f.name,
                f[Fluxion::API::Data::EFilterFlag::IsActive],
                components),
            ctx);
    }
};

template <>
struct std::formatter<Fluxion::API::Data::FiltersTab> : std::formatter<std::string_view>
{
    auto format(const Fluxion::API::Data::FiltersTab& tab, format_context& ctx) const
    {
        const auto& filters = tab.filters.GetBack();
        return std::formatter<std::string_view>::format(
            std::format(
                "Tab(Name: '{}', Active: {}, Filters: {})",
                tab.name,
                tab[Fluxion::API::Data::EFiltersTabFlag::IsActive],
                filters),
            ctx);
    }
};
