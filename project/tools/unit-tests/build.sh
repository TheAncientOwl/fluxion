#!/usr/bin/env bash

set -e

BUILD_DIR="build"
DO_BUILD=false
DO_RUN=false
TARGET=""

usage() {
    echo "Usage: $0 [-b] [-r] [-d <build_dir>] <target_name_or_path>"
    echo "  -b, --build       Build the unit test target"
    echo "  -r, --run         Run the unit test target"
    echo "  -d, --dir <dir>   Build directory (default: build)"
    echo "  -h, --help        Show this help message"
    exit 1
}

# Parse options
while [[ $# -gt 0 ]]; do
    case "$1" in
        -b|--build)
            DO_BUILD=true
            shift
            ;;
        -r|--run)
            DO_RUN=true
            shift
            ;;
        -d|--dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        -*)
            echo "Unknown option: $1"
            usage
            ;;
        *)
            TARGET="$1"
            shift
            ;;
    esac
done

if [[ -z "$TARGET" ]]; then
    echo "Error: Unit test target or path is required."
    usage
fi

# Extract target name if a path was passed
TARGET_NAME=$(basename "$TARGET")

# Build phase
if [[ "$DO_BUILD" == true ]]; then
    echo "==> Building target: ${TARGET_NAME} in ./${BUILD_DIR}"
    cmake --build "${BUILD_DIR}" --target "${TARGET_NAME}"
fi

# Run phase
if [[ "$DO_RUN" == true ]]; then
    echo "==> Running unit tests matching: ${TARGET_NAME}"
    
    # Try finding exact executable in build tree first
    EXE_PATH=$(find "${BUILD_DIR}" -type f -name "${TARGET_NAME}" -perm +111 2>/dev/null | head -n 1)

    if [[ -n "$EXE_PATH" && -x "$EXE_PATH" ]]; then
        echo "==> Executing binary directly: ${EXE_PATH}"
        "$EXE_PATH" "${@}" # Passes through any extra gtest flags
    else
        echo "==> Binary not found directly, falling back to ctest..."
        ctest --test-dir "${BUILD_DIR}" -R "^${TARGET_NAME}$" --output-on-failure
    fi
fi
