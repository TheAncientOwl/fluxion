add_library(fluxion_compile_options INTERFACE)

target_compile_options(fluxion_compile_options INTERFACE
    -fdiagnostics-color
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion
    -Wshadow -Wformat=2 -Wundef
    -Wnon-virtual-dtor -Wold-style-cast
    -Woverloaded-virtual -Wnull-dereference
    -Wdouble-promotion -Wimplicit-fallthrough
    -Wno-error=unused-parameter
)

target_compile_options(fluxion_compile_options INTERFACE
    $<$<CXX_COMPILER_ID:Clang>:
        -Wno-padded
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-weak-vtables
    >
)

target_compile_options(fluxion_compile_options INTERFACE
    $<$<CONFIG:Debug>:
        -O1 -g -fno-omit-frame-pointer
        # -fsanitize=address,undefined
    >
    $<$<CONFIG:Release>:
        -O3 -DNDEBUG
    >
)

target_link_options(fluxion_compile_options INTERFACE
    $<$<CONFIG:Debug>:
        # -fsanitize=address,undefined
    >
)
