#!/usr/bin/env bash
#
# Build every chapter and run its tests, the same way CI does.

set -euo pipefail

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
