/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion/Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief General data.
///

#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Graphite/Common/TWithFlags.hpp"
#include "Graphite/Common/UniqueID.hpp"

#include "imgui.h"

namespace Fluxion::API::Data {

namespace Plugin {

struct OnEnableData
{
};

struct OnDisableData
{
};

}; // namespace Plugin

using LogsTableHeader = std::vector<std::string>;

// clang-format off
enum class EFilterComponentFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsRegex         = 1 << 0, // 00000001
    IsEquals        = 1 << 1, // 00000010
    IsCaseSensitive = 1 << 2, // 00000100
};
// clang-format on

struct FilterComponent : public Graphite::Common::TWithFlags<FilterComponent, EFilterComponentFlag>
{
    Graphite::Common::UniqueID id{};
    Graphite::Common::UniqueID over_field_id{};
    std::string data{};

    FilterComponent() = default;
    FilterComponent(FilterComponent const& other);
    FilterComponent& operator=(FilterComponent const& other);
    FilterComponent(FilterComponent&& other) noexcept = default;
    FilterComponent& operator=(FilterComponent&& other) noexcept = default;
    ~FilterComponent() = default;

    friend std::ostream& operator<<(std::ostream& os, const FilterComponent& v);
};

struct FilterColors
{
    ImVec4 foreground{};
    ImVec4 background{};
};

// clang-format off
enum class EFilterFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsActive        = 1 << 0, // 00000001
    IsHighlightOnly = 1 << 1, // 00000010
    IsCollapsed     = 1 << 2, // 00000100
};
// clang-format on

struct Filter : public Graphite::Common::TWithFlags<Filter, EFilterFlag>
{
    Graphite::Common::UniqueID id{};
    std::string name{};
    std::vector<std::shared_ptr<FilterComponent>> components{};
    FilterColors colors{};
    std::uint8_t priority{};

    Filter() = default;
    Filter(Filter const& other);
    Filter& operator=(Filter const& other);
    Filter(Filter&& other) noexcept;
    Filter& operator=(Filter&& other) noexcept;
    ~Filter() = default;

    friend std::ostream& operator<<(std::ostream& os, const Filter& v);
};

// clang-format off
enum class EFiltersTabFlag : std::uint8_t
{
    None     = 0,     // 00000000
    IsActive = 1 << 0 // 00000001
};
// clang-format on

struct FiltersTab : public Graphite::Common::TWithFlags<FiltersTab, EFiltersTabFlag>
{
    Graphite::Common::UniqueID id{};
    std::string name{};
    std::vector<std::shared_ptr<Filter>> filters{};

    FiltersTab() = default;
    FiltersTab(FiltersTab const& other);
    FiltersTab& operator=(FiltersTab const& other);
    FiltersTab(FiltersTab&& other) noexcept = default;
    FiltersTab& operator=(FiltersTab&& other) noexcept = default;
    ~FiltersTab() = default;

    friend std::ostream& operator<<(std::ostream& os, const FiltersTab& v);
};

using FiltersTabs = std::vector<std::shared_ptr<FiltersTab>>;

} // namespace Fluxion::API::Data
