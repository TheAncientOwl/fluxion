/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DynamicLibrary.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation of @see DynamicLibrary.hpp.
///

#include <filesystem>

#include "Graphite/Common/Plugin/DynamicLibrary.hpp"

namespace Graphite::Common::Plugin {

#if defined(_WIN32)
#include <windows.h>
using LibHandle = HMODULE;
#else
#include <dlfcn.h>
using LibHandle = void*;
#endif

DynamicLibrary::DynamicLibrary(const char* path)
{
#if defined(_WIN32)
    handle = LoadLibraryA(path.c_str());
#else
    handle = dlopen(path, RTLD_LAZY);
#endif
}

DynamicLibrary::DynamicLibrary(std::string_view const path) : DynamicLibrary(path.data())
{
}

DynamicLibrary::DynamicLibrary(const std::filesystem::path& path) : DynamicLibrary(path.c_str())
{
}

DynamicLibrary::~DynamicLibrary()
{
    if (handle)
    {
#if defined(_WIN32)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
    }
}

void* DynamicLibrary::getSymbol(std::string_view const name)
{
#if defined(_WIN32)
    return (void*)GetProcAddress(handle, name.data());
#else
    return dlsym(handle, name.data());
#endif
}

bool DynamicLibrary::isLoaded() const
{
    return handle != nullptr;
}

} // namespace Graphite::Common::Plugin
