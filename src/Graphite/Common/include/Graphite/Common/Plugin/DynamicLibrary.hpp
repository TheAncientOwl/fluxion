/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DynamicLibrary.hpp
/// @author Alexandru Delegeanu
/// @version 1.1
/// @brief DLL utility class.
///

#pragma once

#include <filesystem>

namespace Graphite::Common::Plugin {

#if defined(_WIN32)
#include <windows.h>
using LibHandle = HMODULE;
#else
#include <dlfcn.h>
using LibHandle = void*;
#endif

class DynamicLibrary
{
private:
    DynamicLibrary(const char* path);

public:
    DynamicLibrary(std::string_view const path);
    DynamicLibrary(std::filesystem::path const& path);

    ~DynamicLibrary();

    void* getSymbol(std::string_view const name) const;
    bool isLoaded() const;

private:
    LibHandle handle;
};

} // namespace Graphite::Common::Plugin
