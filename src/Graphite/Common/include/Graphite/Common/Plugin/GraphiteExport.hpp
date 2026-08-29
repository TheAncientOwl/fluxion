/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file GraphiteExport.hpp
/// @author Alexandru Delegeanu
/// @version 1.1
/// @brief .
///

#pragma once

#if defined(_WIN32)
#ifdef GRAPHITE_EXPORTS
#define GRAPHITE_EXPORT __declspec(dllexport)
#else
#define GRAPHITE_EXPORT __declspec(dllimport)
#endif
#else
#define GRAPHITE_EXPORT __attribute__((visibility("default")))
#endif
