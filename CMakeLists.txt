cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

include(project/cmake/compile-options.cmake)
include(project/cmake/precompiled-headers.cmake)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(APPLE)
    project(Fluxion LANGUAGES C CXX OBJC OBJCXX)

    find_program(CLANG_PATH clang)
    find_program(CLANGXX_PATH clang++)

    if(CLANG_PATH AND CLANGXX_PATH)
        set(CMAKE_C_COMPILER ${CLANG_PATH} CACHE FILEPATH "Use Apple Clang for C" FORCE)
        set(CMAKE_CXX_COMPILER ${CLANGXX_PATH} CACHE FILEPATH "Use Apple Clang for C++" FORCE)
    endif()
else()
    project(Fluxion LANGUAGES C CXX)
endif()

add_subdirectory(external)
add_subdirectory(src/Api)
add_subdirectory(src/Graphite)
add_subdirectory(src/Application)

# Create executable
set(SOURCES
    src/main.cpp
)

if (APPLE)
    add_executable(fluxion MACOSX_BUNDLE ${SOURCES})

    # Copy the icon to Resources folder before the bundle is finalized
    set(ICON_SRC ${CMAKE_SOURCE_DIR}/assets/icons/fluxion725x725.icns)
    set(ICON_DST $<TARGET_FILE_DIR:fluxion>/../Resources/fluxion725x725.icns)

    add_custom_command(TARGET fluxion PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:fluxion>/../Resources
        COMMAND ${CMAKE_COMMAND} -E copy ${ICON_SRC} ${ICON_DST}
        COMMENT "Copying fluxion icon to Resources folder before build"
    )

    # Set the icon filename only
    set_target_properties(fluxion PROPERTIES
        MACOSX_BUNDLE_ICON_FILE fluxion725x725.icns
    )

    # Copy the rest of the assets folder after build
    add_custom_command(TARGET fluxion POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:fluxion>/../Resources/assets
        COMMENT "Copying remaining assets folder to Resources directory"
    )
else()
    add_executable(fluxion ${SOURCES})
endif()

target_compile_definitions(fluxion PRIVATE
    $<$<CONFIG:Release>:GRAPHITE_NO_LOGGER>
)

set_target_properties(fluxion PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_link_libraries(fluxion PRIVATE
    fluxion_app
    fluxion_compile_options
    fluxion_pch
)

# target_precompile_headers(fluxion REUSE_FROM fluxion_pch)

# Include directories
target_include_directories(fluxion PRIVATE
    external
    src
)

# Copy assets to build folder
add_custom_command(TARGET fluxion POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:fluxion>/assets
    COMMENT "Copying assets folder to build directory"
)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(fluxion copy_assets)
