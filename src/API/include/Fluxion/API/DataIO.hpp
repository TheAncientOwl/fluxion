/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DataIO.hpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief IO related utilities.
///

#pragma once

#include <format>
#include <sstream>

#include "Fluxion/API/Data.hpp"

namespace std {

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<T>& ptr)
{
    if (ptr)
        os << *ptr;
    else
        os << "nullptr";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    os << "[";
    bool first = true;
    for (auto const& v : vec)
    {
        if (!first)
            os << ", ";
        os << v;
        first = false;
    }
    os << "]";
    return os;
}

} // namespace std

namespace Fluxion::Utils::Format {

template <typename T>
std::string format_vector(const std::vector<T>& vec)
{
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (auto const& elem : vec)
    {
        if (!first)
            oss << ", ";
        oss << elem;
        first = false;
    }
    oss << "]";
    return oss.str();
}

} // namespace Fluxion::Utils::Format

namespace Fluxion::API::Data {

inline std::ostream& operator<<(std::ostream& os, const FilterComponent& v)
{
    std::string flags_str;
    bool first = true;
    if (v[EFilterComponentFlag::IsRegex])
    {
        if (!first)
            flags_str += ", ";
        flags_str += "IsRegex";
        first = false;
    }
    if (v[EFilterComponentFlag::IsEquals])
    {
        if (!first)
            flags_str += ", ";
        flags_str += "IsEquals";
        first = false;
    }
    if (v[EFilterComponentFlag::IsCaseSensitive])
    {
        if (!first)
            flags_str += ", ";
        flags_str += "IsCaseSensitive";
        first = false;
    }
    return os << "FilterComponent(id=" << v.id << ", over_field_id=" << v.over_field_id
              << ", data=\"" << v.data << "\", flags=" << flags_str << ")";
}

inline std::ostream& operator<<(std::ostream& os, const FilterColors& v)
{
    return os << "FilterColors(fg=(" << v.foreground.x << "," << v.foreground.y << ","
              << v.foreground.z << "," << v.foreground.w << "), bg=(" << v.background.x << ","
              << v.background.y << "," << v.background.z << "," << v.background.w << "))";
}

inline std::ostream& operator<<(std::ostream& os, const Filter& v)
{
    std::string flags_str;
    bool first = true;
    if (v[EFilterFlag::IsActive])
    {
        if (!first)
            flags_str += ", ";
        flags_str += "IsActive";
        first = false;
    }
    if (v[EFilterFlag::IsHighlightOnly])
    {
        if (!first)
            flags_str += ", ";
        flags_str += "IsHighlightOnly";
        first = false;
    }
    if (v[EFilterFlag::IsCollapsed])
    {
        if (!first)
            flags_str += ", ";
        flags_str += "IsCollapsed";
        first = false;
    }
    os << "Filter(id=" << v.id << ", name=\"" << v.name
       << "\", components={.back=" << v.components.back << " | .front=" << v.components.front
       << ", priority=" << +v.priority << ", colors=" << v.colors << ", flags=" << flags_str << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const FiltersTab& v)
{
    std::string flags_str;
    bool first = true;
    if (v[EFiltersTabFlag::IsActive])
    {
        if (!first)
            flags_str += ", ";
        flags_str += "IsActive";
        first = false;
    }
    os << "FiltersTab(id=" << v.id << ", name=\"" << v.name << "\", filters={.back=" << v.filters.back
       << " | .front=" << v.filters.front << ", flags=" << flags_str << ")";
    return os;
}

} // namespace Fluxion::API::Data

template <>
struct std::formatter<Fluxion::API::Data::FilterComponent>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Fluxion::API::Data::FilterComponent& fc, FormatContext& ctx)
    {
        std::string flags_str;
        bool first = true;
        if (fc[Fluxion::API::Data::EFilterComponentFlag::IsRegex])
        {
            flags_str += "IsRegex";
            first = false;
        }
        if (fc[Fluxion::API::Data::EFilterComponentFlag::IsEquals])
        {
            if (!first)
                flags_str += ", ";
            flags_str += "IsEquals";
            first = false;
        }
        if (fc[Fluxion::API::Data::EFilterComponentFlag::IsCaseSensitive])
        {
            if (!first)
                flags_str += ", ";
            flags_str += "IsCaseSensitive";
            first = false;
        }

        auto out = ctx.out();
        out = std::format_to(out, "FilterComponent(id=");
        out = std::format_to(out, "{}", fc.id);
        out = std::format_to(out, ", over_field_id={}", fc.over_field_id);
        out = std::format_to(out, ", data=\"{}\"", fc.data);
        out = std::format_to(out, ", flags={}", flags_str);
        out = std::format_to(out, ")");
        return out;
    }
};

template <>
struct std::formatter<Fluxion::API::Data::FilterColors>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Fluxion::API::Data::FilterColors& fc, FormatContext& ctx)
    {
        auto out = ctx.out();
        out = std::format_to(out, "FilterColors(fg=(");
        out = std::format_to(
            out, "{}, {}, {}, {}", fc.foreground.x, fc.foreground.y, fc.foreground.z, fc.foreground.w);
        out = std::format_to(out, "), bg=(");
        out = std::format_to(
            out, "{}, {}, {}, {}", fc.background.x, fc.background.y, fc.background.z, fc.background.w);
        out = std::format_to(out, "))");
        return out;
    }
};

template <>
struct std::formatter<Fluxion::API::Data::Filter>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Fluxion::API::Data::Filter& f, FormatContext& ctx)
    {
        std::string flags_str;
        bool first = true;
        if (f[Fluxion::API::Data::EFilterFlag::IsActive])
        {
            if (!first)
                flags_str += ", ";
            flags_str += "IsActive";
            first = false;
        }
        if (f[Fluxion::API::Data::EFilterFlag::IsHighlightOnly])
        {
            if (!first)
                flags_str += ", ";
            flags_str += "IsHighlightOnly";
            first = false;
        }
        if (f[Fluxion::API::Data::EFilterFlag::IsCollapsed])
        {
            if (!first)
                flags_str += ", ";
            flags_str += "IsCollapsed";
            first = false;
        }
        auto out = ctx.out();
        out = std::format_to(out, "Filter(id=");
        out = std::format_to(out, "{}", f.id);
        out = std::format_to(out, ", name=\"{}\"", f.name);
        out = std::format_to(out, ", components.back={}", f.components.back.size());
        out = std::format_to(out, ", components.front={}", f.components.front.size());
        out = std::format_to(out, ", priority={}", +f.priority);
        out = std::format_to(out, ", flags={}", flags_str);
        out = std::format_to(out, ")");
        return out;
    }
};

template <>
struct std::formatter<Fluxion::API::Data::FiltersTab>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Fluxion::API::Data::FiltersTab& ft, FormatContext& ctx)
    {
        std::string flags_str;
        bool first = true;
        if (ft[Fluxion::API::Data::EFiltersTabFlag::IsActive])
        {
            if (!first)
                flags_str += ", ";
            flags_str += "IsActive";
            first = false;
        }
        auto out = ctx.out();
        out = std::format_to(out, "FiltersTab(id=");
        out = std::format_to(out, "{}", ft.id);
        out = std::format_to(out, ", name=\"{}\"", ft.name);
        out = std::format_to(out, ", filters.back={}", ft.filters.back.size());
        out = std::format_to(out, ", filters.front={}", ft.filters.front.size());
        out = std::format_to(out, ", flags={}", flags_str);
        out = std::format_to(out, ")");
        return out;
    }
};
