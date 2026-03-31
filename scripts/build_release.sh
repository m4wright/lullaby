#!/usr/bin/bash
set -euo pipefail

cd release
cmake -DCMAKE_BUILD_TYPE=Release ..

# This will take up to ~10 minutes depending on what needs to be recompiled. It builds with O2
time make

# Run the tests
ctest

