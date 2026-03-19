/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersTabs.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see Data.hpp::FiltersTabs.
///

#include "Data.hpp"

#include "Core/Logger/Logger.hpp"

#include <algorithm>
#include <iterator>

namespace Fluxion::API::Data {

// ---------------- FilterComponent ----------------

FilterComponent::FilterComponent(FilterComponent const& other)
    : Internal::TWithFlags<FilterComponent, EFilterComponentFlag>(other)
    , id(Graphite::Core::Common::UniqueID::Generate())
    , over_field_id(other.over_field_id)
    , data(other.data)
{
}

FilterComponent& FilterComponent::operator=(FilterComponent const& other)
{
    if (this == &other)
    {
        return *this;
    }

    Internal::TWithFlags<FilterComponent, EFilterComponentFlag>::operator=(other);
    id = Graphite::Core::Common::UniqueID::Generate();
    over_field_id = other.over_field_id;
    data = other.data;

    return *this;
}

// ---------------- Filter ----------------

Filter::Filter(Filter const& other)
    : Internal::TWithFlags<Filter, EFilterFlag>(other)
    , id(Graphite::Core::Common::UniqueID::Generate())
    , name(other.name)
    , colors(other.colors)
    , priority(other.priority)
{
    components.reserve(other.components.size());
    std::transform(
        other.components.begin(),
        other.components.end(),
        std::back_inserter(components),
        [](auto const& component) { return std::make_shared<FilterComponent>(*component); });
}

Filter& Filter::operator=(Filter const& other)
{
    if (this == &other)
    {
        return *this;
    }

    Internal::TWithFlags<Filter, EFilterFlag>::operator=(other);
    id = Graphite::Core::Common::UniqueID::Generate();
    name = other.name;
    colors = other.colors;
    priority = other.priority;

    components.clear();
    components.reserve(other.components.size());
    std::transform(
        other.components.begin(),
        other.components.end(),
        std::back_inserter(components),
        [](auto const& component) { return std::make_shared<FilterComponent>(*component); });

    return *this;
}

// ---------------- FiltersTab ----------------

FiltersTab::FiltersTab(FiltersTab const& other)
    : Internal::TWithFlags<FiltersTab, EFiltersTabFlag>(other)
    , id(Graphite::Core::Common::UniqueID::Generate())
    , name(other.name)
{
    filters.reserve(other.filters.size());
    std::transform(
        other.filters.begin(), other.filters.end(), std::back_inserter(filters), [](auto const& filter) {
            return std::make_shared<Filter>(*filter);
        });
}

FiltersTab& FiltersTab::operator=(FiltersTab const& other)
{
    if (this == &other)
    {
        return *this;
    }

    Internal::TWithFlags<FiltersTab, EFiltersTabFlag>::operator=(other);
    id = Graphite::Core::Common::UniqueID::Generate();
    name = other.name;

    filters.clear();
    filters.reserve(other.filters.size());
    std::transform(
        other.filters.begin(), other.filters.end(), std::back_inserter(filters), [](auto const& filter) {
            return std::make_shared<Filter>(*filter);
        });

    return *this;
}

} // namespace Fluxion::API::Data
