/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersTabs.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see Fluxion/Data.hpp::FiltersTabs.
///

#include "Fluxion/API/Data.hpp"

#include <algorithm>
#include <iterator>

namespace Fluxion::API::Data {

// ---------------- FilterComponent ----------------

FilterComponent::FilterComponent(FilterComponent const& other)
    : Graphite::Common::TWithFlags<FilterComponent, EFilterComponentFlag>(other)
    , id(Graphite::Common::UniqueID::Generate())
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

    Graphite::Common::TWithFlags<FilterComponent, EFilterComponentFlag>::operator=(other);
    id = Graphite::Common::UniqueID::Generate();
    over_field_id = other.over_field_id;
    data = other.data;

    return *this;
}

// ---------------- Filter ----------------

Filter::Filter(Filter const& other)
    : Graphite::Common::TWithFlags<Filter, EFilterFlag>(other)
    , id(Graphite::Common::UniqueID::Generate())
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

    Graphite::Common::TWithFlags<Filter, EFilterFlag>::operator=(other);
    id = Graphite::Common::UniqueID::Generate();
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
    : Graphite::Common::TWithFlags<FiltersTab, EFiltersTabFlag>(other)
    , id(Graphite::Common::UniqueID::Generate())
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

    Graphite::Common::TWithFlags<FiltersTab, EFiltersTabFlag>::operator=(other);
    id = Graphite::Common::UniqueID::Generate();
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
