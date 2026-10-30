#!/usr/bin/env bash
# Builds the project in release mode and runs all unit tests:
# utcommon (common library), utaos (AOS version) and utsoa (SOA version).
# Usage: ./utest.sh   (set CXX to choose another compiler; default: g++-15 when installed)
set -euo pipefail

cd "$(dirname "$0")"

# The project is compiled with g++-15 (C++23). Use it unless another compiler is requested;
# on macOS, Homebrew's g++ also needs the SDK path to link
if [[ -z "${CXX:-}" ]] && command -v g++-15 > /dev/null; then
  export CXX=g++-15
  if [[ "$(uname)" == "Darwin" ]]; then export SDKROOT="${SDKROOT:-$(xcrun --show-sdk-path)}"; fi
fi

cmake --preset default
cmake --build --preset release

build_dir="out/build/default"
failed=0
for test in utcommon utaos utsoa; do
  echo "=== Running ${test} ==="
  if ! "${build_dir}/${test}/Release/${test}"; then
    failed=1
  fi
done

if [[ ${failed} -ne 0 ]]; then
  echo "Some unit tests FAILED"
  exit 1
fi
echo "All unit tests passed"
