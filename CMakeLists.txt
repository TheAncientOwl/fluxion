cmake_minimum_required(VERSION 3.25)

# --- 1. Project Setup ---
# Define project and languages first. 
# On Apple, we need OBJC/OBJCXX for Metal and Cocoa.
if(APPLE)
    project(Fluxion LANGUAGES C CXX OBJC OBJCXX)
else()
    project(Fluxion LANGUAGES C CXX)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static linking for MinGW/GCC on Windows
if(WIN32 AND NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# --- 2. Build Logic & Subdirectories ---
include(project/cmake/compile-options.cmake)
include(project/cmake/precompiled-headers.cmake)

add_subdirectory(external)
add_subdirectory(src/Api)
add_subdirectory(src/Graphite)
add_subdirectory(src/Application)

# --- 3. Executable Configuration ---
set(SOURCES src/main.cpp)

if(APPLE)
    # Define the icon as a resource
    set(MACOS_ICON ${CMAKE_SOURCE_DIR}/assets/icons/fluxion725x725.icns)

    # Adding the icon to the executable sources tells CMake it's part of the bundle
    add_executable(fluxion MACOSX_BUNDLE ${SOURCES} ${MACOS_ICON})

    set_target_properties(fluxion PROPERTIES
        MACOSX_BUNDLE_ICON_FILE fluxion725x725.icns
        RESOURCE "${MACOS_ICON}"
    )
else()
    add_executable(fluxion ${SOURCES})
endif()

# --- 4. Target Properties & Linking ---
target_compile_definitions(fluxion PRIVATE
    $<$<CONFIG:Release>:GRAPHITE_NO_LOGGER>
)

# Set output directories
set_target_properties(fluxion PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(fluxion PRIVATE
    fluxion_compile_options
    fluxion_app
)

target_include_directories(fluxion PRIVATE
    external
    src
)

# --- 5. Asset Handling ---

# A: Base assets for Windows/Linux (copies to the folder where the .exe lives)
if(NOT APPLE)
    add_custom_command(TARGET fluxion POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:fluxion>/assets
        COMMENT "Copying assets to executable directory"
    )
endif()

# B: Assets for macOS Bundle (copies into Fluxion.app/Contents/Resources)
if(APPLE)
    add_custom_command(TARGET fluxion POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:fluxion>/../Resources/assets
        COMMENT "Copying assets into macOS App Bundle"
    )
endif()

# C: Development Sync (keeps build folder root updated for VS Code/General use)
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Syncing assets to build root"
)
add_dependencies(fluxion copy_assets)
