#!/usr/bin/env bash
set -euo pipefail
project_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
test_build=$(mktemp -d)
trap 'rm -rf -- "$test_build"' EXIT
test_flags=(-std=c++17 -Wall -Wextra -Wpedantic -Werror -O2)
if [[ ${SANITIZE:-0} == 1 ]]; then
    test_flags+=(-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
fi
"${CXX:-g++}" "${test_flags[@]}" -I"$project_root" -I"$project_root/back" \
    "$project_root/tests/core_tests.cpp" "$project_root/store.cpp" \
    "$project_root/order.cpp" "$project_root/statistics.cpp" \
    "$project_root/data_generator.cpp" "$project_root/internal/model_support.cpp" \
    "$project_root/internal/supply_manager.cpp" \
    -o "$test_build/core_tests"
"$test_build/core_tests"
