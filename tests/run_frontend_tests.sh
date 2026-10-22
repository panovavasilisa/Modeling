#!/usr/bin/env bash
set -euo pipefail
project_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
test_build=$(mktemp -d)
trap 'rm -rf -- "$test_build"' EXIT
cd "$test_build"
# Пути проекта qmake задаются относительно корня исходников.
qmake6 "$project_root/PharmDelivery.pro" -after 'QT+=testlib' 'CONFIG+=testcase' 'TARGET=frontend_tests' 'SOURCES-=main.cpp' "SOURCES+=$project_root/tests/frontend_tests.cpp"
make -j2
QT_QPA_PLATFORM=offscreen ./frontend_tests
