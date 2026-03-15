/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file Export.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief .
///

#pragma once

#if defined(_WIN32)
#define GRAPHITE_EXPORT __declspec(dllexport)
#else
#define GRAPHITE_EXPORT __attribute__((visibility("default")))
#endif
