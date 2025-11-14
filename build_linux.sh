#!/usr/bin/env bash
# build_linux.sh - Configure and build the project on Linux (CMake + Qt)
# Usage:
#   ./build_linux.sh                 # default: Debug build using Ninja (if available)
#   ./build_linux.sh -t Release      # Release build
#   ./build_linux.sh -t Debug -j 8   # set parallel jobs
#   ./build_linux.sh -c              # clean build directory
#   ./build_linux.sh -i /usr/local   # install after build (requires sudo for system prefixes)
#   ./build_linux.sh -r              # run the produced binary (only for Desktop builds)

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build/linux"
BUILD_TYPE="Debug"
JOBS="$(nproc)"
CLEAN=0
INSTALL_PREFIX=""
RUN_AFTER=0
GENERATOR=""

show_help() {
    sed -n '1,120p' "$0" | sed -n '1,120p'
}

while getopts ":t:j:ci:rh" opt; do
  case $opt in
    t) BUILD_TYPE="$OPTARG" ;;
    j) JOBS="$OPTARG" ;;
    c) CLEAN=1 ;;
    i) INSTALL_PREFIX="$OPTARG" ;;
    r) RUN_AFTER=1 ;;
    h) show_help; exit 0 ;;
    \?) echo "Invalid option -$OPTARG" >&2; show_help; exit 1 ;;
  esac
done

echo "Project root: $ROOT_DIR"
echo "Build dir: $BUILD_DIR"
echo "Build type: $BUILD_TYPE"

# Choose generator: prefer Ninja if available
if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
elif command -v cmake >/dev/null 2>&1; then
    # fallback to default (Unix Makefiles)
    GENERATOR="Unix Makefiles"
else
    echo "ERROR: cmake is required but not found in PATH." >&2
    exit 1
fi

echo "Using CMake generator: $GENERATOR"

if [ "$CLEAN" -eq 1 ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
CMAKE_ARGS=(
    -S "$ROOT_DIR"
    -B "$BUILD_DIR"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
)

if [ -n "$GENERATOR" ]; then
    CMAKE_ARGS+=( -G "$GENERATOR" )
fi

echo "Running: cmake ${CMAKE_ARGS[*]}"
cmake "${CMAKE_ARGS[@]}"

# Build
echo "Building (jobs=$JOBS)..."
if [ "$GENERATOR" = "Ninja" ]; then
    cmake --build . --config "$BUILD_TYPE" -j "$JOBS"
else
    cmake --build . --config "$BUILD_TYPE" -- -j"$JOBS"
fi

# Optionally install
if [ -n "$INSTALL_PREFIX" ]; then
    echo "Installing to $INSTALL_PREFIX"
    if [ "$EUID" -ne 0 ] && [[ "$INSTALL_PREFIX" == /* ]]; then
        echo "Installing to system prefix requires sudo. Running with sudo..."
        sudo cmake --install . --prefix "$INSTALL_PREFIX"
    else
        cmake --install . --prefix "$INSTALL_PREFIX"
    fi
fi

# Optionally run (attempt to run the built desktop binary)
if [ "$RUN_AFTER" -eq 1 ]; then
    # Try to find the built binary in the build dir
    echo "Attempting to run the built binary..."
    # Common output paths
    possible=("$BUILD_DIR/modern_setting_app" "$BUILD_DIR/modern_setting_app" "$BUILD_DIR/Debug/modern_setting_app" "$BUILD_DIR/Release/modern_setting_app")
    ran=0
    for p in "${possible[@]}"; do
        if [ -x "$p" ]; then
            echo "Running $p"
            "$p" &
            ran=1
            break
        fi
    done
    if [ "$ran" -eq 0 ]; then
        echo "Could not find executable to run. Search the build tree for 'modern_setting_app'"
        find "$BUILD_DIR" -maxdepth 3 -type f -executable -name 'modern_setting_app' || true
    fi
fi

echo "Build finished." 
