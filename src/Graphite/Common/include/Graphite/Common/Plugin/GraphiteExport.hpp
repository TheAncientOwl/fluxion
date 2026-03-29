/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GraphiteExport.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief .
///

#pragma once

#if defined(_WIN32)
#define GRAPHITE_EXPORT __declspec(dllexport)
#else
#define GRAPHITE_EXPORT __attribute__((visibility("default")))
#endif
