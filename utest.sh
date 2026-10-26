#!/usr/bin/env bash
# Builds the project in release mode and runs all unit tests:
# utcommon (common library), utaos (AOS version) and utsoa (SOA version).
# Usage: ./utest.sh
set -euo pipefail

cd "$(dirname "$0")"

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
