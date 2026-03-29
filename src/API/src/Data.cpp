/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersTabs.cpp
/// @author Alexandru Delegeanu
/// @version 0.3
/// @brief Implementation of @see Fluxion/Data.hpp::FiltersTabs.
///

#include "Fluxion/API/Data.hpp"

namespace Fluxion::API::Data {

void FiltersTab::UpdateImGuiID()
{
    imgui_id = name + "###" + id.ToString();
}

// // ---------------- FilterComponent ----------------

// FilterComponent::FilterComponent() : id(Graphite::Common::Utility::UniqueID::Generate())
// {
// }

// FilterComponent::FilterComponent(FilterComponent const& other)
//     : Graphite::Common::Utility::TWithFlags<FilterComponent, EFilterComponentFlag>(other)
//     , id(Graphite::Common::Utility::UniqueID::Generate())
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

//     Graphite::Common::Utility::TWithFlags<FilterComponent, EFilterComponentFlag>::operator=(other);
//     id = Graphite::Common::Utility::UniqueID::Generate();
//     over_field_id = other.over_field_id;
//     data = other.data;

//     return *this;
// }

// // ---------------- Filter ----------------

// Filter::Filter() : id(Graphite::Common::Utility::UniqueID::Generate())
// {
// }

// Filter::Filter(Filter const& other)
//     : Graphite::Common::Utility::TWithFlags<Filter, EFilterFlag>(other)
//     , id(Graphite::Common::Utility::UniqueID::Generate())
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

//     Graphite::Common::Utility::TWithFlags<Filter, EFilterFlag>::operator=(other);
//     id = Graphite::Common::Utility::UniqueID::Generate();
//     name = other.name;
//     colors = other.colors;
//     priority = other.priority;
//     components = other.components;

//     return *this;
// }

// // ---------------- FiltersTab ----------------

// FiltersTab::FiltersTab() : id(Graphite::Common::Utility::UniqueID::Generate())
// {
// }

// FiltersTab::FiltersTab(FiltersTab const& other)
//     : Graphite::Common::Utility::TWithFlags<FiltersTab, EFiltersTabFlag>(other)
//     , id(Graphite::Common::Utility::UniqueID::Generate())
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

//     Graphite::Common::Utility::TWithFlags<FiltersTab, EFiltersTabFlag>::operator=(other);
//     id = Graphite::Common::Utility::UniqueID::Generate();
//     name = other.name;
//     filters = other.filters;

//     return *this;
// }

} // namespace Fluxion::API::Data
