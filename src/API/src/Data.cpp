// /// --------------------------------------------------------------------------
// ///                     Copyright (c) by Fluxion 2026
// /// --------------------------------------------------------------------------
// /// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
// ///
// /// @file FiltersTabs.cpp
// /// @author Alexandru Delegeanu
// /// @version 0.2
// /// @brief Implementation of @see Fluxion/Data.hpp::FiltersTabs.
// ///

// #include "Fluxion/API/Data.hpp"

// namespace Fluxion::API::Data {

// // ---------------- FilterComponent ----------------

// FilterComponent::FilterComponent() : id(Graphite::Common::UniqueID::Generate())
// {
// }

// FilterComponent::FilterComponent(FilterComponent const& other)
//     : Graphite::Common::TWithFlags<FilterComponent, EFilterComponentFlag>(other)
//     , id(Graphite::Common::UniqueID::Generate())
//     , over_field_id(other.over_field_id)
//     , data(other.data)
// {
// }

// FilterComponent& FilterComponent::operator=(FilterComponent const& other)
// {
//     if (this == &other)
//     {
//         return *this;
//     }

//     Graphite::Common::TWithFlags<FilterComponent, EFilterComponentFlag>::operator=(other);
//     id = Graphite::Common::UniqueID::Generate();
//     over_field_id = other.over_field_id;
//     data = other.data;

//     return *this;
// }

// // ---------------- Filter ----------------

// Filter::Filter() : id(Graphite::Common::UniqueID::Generate())
// {
// }

// Filter::Filter(Filter const& other)
//     : Graphite::Common::TWithFlags<Filter, EFilterFlag>(other)
//     , id(Graphite::Common::UniqueID::Generate())
//     , name(other.name)
//     , components(other.components)
//     , colors(other.colors)
//     , priority(other.priority)
// {
// }

// Filter& Filter::operator=(Filter const& other)
// {
//     if (this == &other)
//     {
//         return *this;
//     }

//     Graphite::Common::TWithFlags<Filter, EFilterFlag>::operator=(other);
//     id = Graphite::Common::UniqueID::Generate();
//     name = other.name;
//     colors = other.colors;
//     priority = other.priority;
//     components = other.components;

//     return *this;
// }

// // ---------------- FiltersTab ----------------

// FiltersTab::FiltersTab() : id(Graphite::Common::UniqueID::Generate())
// {
// }

// FiltersTab::FiltersTab(FiltersTab const& other)
//     : Graphite::Common::TWithFlags<FiltersTab, EFiltersTabFlag>(other)
//     , id(Graphite::Common::UniqueID::Generate())
//     , name(other.name)
//     , filters(other.filters)
// {
// }

// FiltersTab& FiltersTab::operator=(FiltersTab const& other)
// {
//     if (this == &other)
//     {
//         return *this;
//     }

//     Graphite::Common::TWithFlags<FiltersTab, EFiltersTabFlag>::operator=(other);
//     id = Graphite::Common::UniqueID::Generate();
//     name = other.name;
//     filters = other.filters;

//     return *this;
// }

// } // namespace Fluxion::API::Data
