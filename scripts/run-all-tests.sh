#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MODE="${1:-Release}"
BUILD="/tmp/sov-build-$MODE"
cmake_args=(-S "$ROOT" -B "$BUILD" -DSOVEREIGN_BUILD_TESTS=ON)
case "$MODE" in
  Debug) cmake_args+=(-DCMAKE_BUILD_TYPE=Debug) ;;
  Release) cmake_args+=(-DCMAKE_BUILD_TYPE=Release) ;;
  ASan)
    cmake_args+=(-DCMAKE_BUILD_TYPE=Debug
      -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g")
    ;;
  UBSan)
    cmake_args+=(-DCMAKE_BUILD_TYPE=Debug
      -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer -g")
    ;;
  *) echo "usage: $0 Debug|Release|ASan|UBSan"; exit 1 ;;
esac
cmake "${cmake_args[@]}"
cmake --build "$BUILD" -j"$(nproc 2>/dev/null || echo 2)"
ctest --test-dir "$BUILD" --output-on-failure --timeout 60 || true
CLI=$(find "$BUILD" -name sovereign-cli -type f -executable | head -1)
if [[ -n "$CLI" ]]; then
  bash "$ROOT/scripts/e2e-temp-dataroot.sh" "$CLI"
fi
echo "run-all-tests $MODE finished"
