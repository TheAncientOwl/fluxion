/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.hpp
/// @author Alexandru Delegeanu
/// @version 5.1
/// @brief Data structs
///

#pragma once

#include <string>

#include <nlohmann/json.hpp>
#include "Graphite/Common/Utility/UniqueID.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::Data {

struct RegexTag
{
    Graphite::Common::Utility::UniqueID id{};
    std::string display_name{};
    std::string regex_data{};
    bool visible{};
};

using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

struct Settings
{
    struct MultithreadingParams
    {
        std::int32_t workers_count{10}; // how many threads
        std::int32_t batch_capacity{5000}; // logs per batch
        std::int32_t available_batches_per_worker{4}; // total available batches
        std::int32_t rows_per_transaction{
            50000}; // how many rows have to be parsed before committing to the database
    };

    struct ImportMultithreadingParams : public MultithreadingParams
    {
        std::int32_t file_target_slice_mb{4};
    };

    ImportMultithreadingParams import_params{};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
    Data::Settings::ImportMultithreadingParams,
    workers_count,
    batch_capacity,
    available_batches_per_worker,
    rows_per_transaction,
    file_target_slice_mb)

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V5::Data
