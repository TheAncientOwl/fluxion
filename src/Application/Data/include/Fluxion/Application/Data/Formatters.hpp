/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Formatters.hpp
/// @author Alexandru Delegeanu
/// @version 0.8
/// @brief IO related utilities.
///

#pragma once

#include <format>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "imgui.h"

#include "Data.hpp"

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

template <typename T>
struct formatter<std::optional<T>> : std::formatter<std::string_view>
{
    auto format(const std::optional<T>& opt, format_context& ctx) const
    {
        std::string s = "";
        if (static_cast<bool>(opt))
        {
            s = std::format("{}", *opt);
        }
        else
        {
            s = "---";
        }
        return std::formatter<std::string_view>::format(s, ctx);
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
struct std::formatter<Fluxion::API::Data::Common::Highlight> : std::formatter<std::string_view>
{
    auto format(const Fluxion::API::Data::Common::Highlight& c, format_context& ctx) const
    {
        return std::formatter<std::string_view>::format(
            std::format("FG:{} BG:{}", c.foreground, c.background), ctx);
    }
};

template <>
struct std::formatter<Fluxion::Application::Data::Filters::Condition>
    : std::formatter<std::string_view>
{
    auto format(const Fluxion::Application::Data::Filters::Condition& fc, format_context& ctx) const
    {
        return std::formatter<std::string_view>::format(
            std::format(
                "Condition(ID: {}, ColumnID: {}, Data: '{}')", fc.id, fc.over_column_id, fc.data),
            ctx);
    }
};

template <>
struct std::formatter<Fluxion::Application::Data::Filters::Filter> : std::formatter<std::string_view>
{
    auto format(const Fluxion::Application::Data::Filters::Filter& f, format_context& ctx) const
    {
        const auto& conditions = f.conditions.GetBack();
        // Now 'std::formatter<vector<shared_ptr<FilterCondition>>>' is "defined" (as a shell)
        // so this line won't trigger an "undefined template" error.
        return std::formatter<std::string_view>::format(
            std::format(
                "Filter(Name: '{}', Active: {}, Conditions: {})",
                f.name,
                f[Fluxion::Application::Data::Filters::EFilterFlag::IsActive],
                conditions),
            ctx);
    }
};

template <>
struct std::formatter<Fluxion::Application::Data::Filters::Tab> : std::formatter<std::string_view>
{
    auto format(const Fluxion::Application::Data::Filters::Tab& tab, format_context& ctx) const
    {
        const auto& filters = tab.filters.GetBack();
        return std::formatter<std::string_view>::format(
            std::format(
                "Tab(Name: '{}', Active: {}, Filters: {})",
                tab.name,
                tab[Fluxion::Application::Data::Filters::ETabFlag::IsActive],
                filters),
            ctx);
    }
};
