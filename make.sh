#!/bin/bash
# Thin wrapper around CMake for incremental builds.
# First invocation generates ./build/ from CMakeLists.txt; subsequent runs
# only recompile changed .cpp files. Output: bin/app.bin
#
# Usage:
#   ./make.sh             # incremental build
#   ./make.sh clean       # wipe build/ and bin/ first
#   ./make.sh -- <args>   # pass extra args to `cmake --build`
#
# Env:
#   JOBS=<n>              # override parallel job count (default: 4)
#
# NEVER pass a bare `-j` to cmake: with the Makefile generator that becomes
# `make -j` = unlimited jobs, which forks one compiler per source file and
# thrashes the machine into swap. Always bound with an explicit integer.
JOBS=4
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

if [[ "${1:-}" == "clean" ]]; then
    rm -rf build bin
    shift
fi

# Bounded parallel job count. Prefer env override, else nproc/sysctl, else 4.
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# Configure. Only does real work on the first run or after CMakeLists changes.
if [[ -f build/CMakeCache.txt ]]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >/dev/null
else
    echo "Configuring (first run)..."
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
fi

# Pass-through args after "--"
extra=()
if [[ "${1:-}" == "--" ]]; then
    shift
    extra=("$@")
fi

cmake --build build -j "$JOBS" "${extra[@]}"
echo "Build OK -> bin/app.bin  (JOBS=$JOBS)"
