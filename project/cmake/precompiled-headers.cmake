if (APPLE)
    add_library(fluxion_pch OBJECT 
        ${CMAKE_CURRENT_LIST_DIR}/pch_dummy.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pch_dummy.mm
    )
else()
    add_library(fluxion_pch OBJECT 
        ${CMAKE_CURRENT_LIST_DIR}/pch_dummy.cpp
    )
endif()

target_compile_features(fluxion_pch PUBLIC cxx_std_23)

if (APPLE)
    set_target_properties(fluxion_pch PROPERTIES
        CXX_EXTENSIONS OFF
        OBJCXX_STANDARD 23
        OBJCXX_STANDARD_REQUIRED ON
    )
else()
    set_target_properties(fluxion_pch PROPERTIES
        CXX_EXTENSIONS OFF
    )
endif()

target_precompile_headers(fluxion_pch PRIVATE
    <algorithm>
    <iterator>
    <memory>
    <iostream>
    <ostream>
    <sstream>
    <string>
    <type_traits>
    <vector>
    <format>
    <filesystem>
    <regex>
    <string_view>
    <unordered_map>
    <optional>
    <random>
    <exception>
    <stdio.h>
    <concepts>
    <functional>
    <unordered_set>
    <utility>
    <cstdint>
    <atomic>
    <cstddef>
    <iomanip>
    <ranges>
    <array>
    <fstream>
    <chrono>
    <condition_variable>
    <mutex>
    <queue>
    <thread>
    <stdlib.h>
)

target_link_libraries(fluxion_pch PUBLIC fluxion_compile_options)
