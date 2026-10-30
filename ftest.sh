#!/usr/bin/env bash
# Functional tests for grav-aos and grav-soa. They run the real executables:
#  - success cases: init_config.txt and final_config.txt must match the expected files
#    in ftest/<case>/ (worked out by hand or provided by the instructor, see ftest/README.md)
#  - error cases: the program must fail with the expected error message
#  - AOS vs SOA: both versions must write identical files for a larger input
#
# Usage: ./ftest.sh                   builds with the default preset, then runs the tests
#        BUILD_DIR=<dir> ./ftest.sh   uses the executables already built in <dir>
#        (set CXX to choose another compiler; default: g++-15 when installed)
set -uo pipefail

cd "$(dirname "$0")"
root=$(pwd)

if [[ -z "${BUILD_DIR:-}" ]]; then
  # The project is compiled with g++-15 (C++23). Use it unless another compiler is requested;
  # on macOS, Homebrew's g++ also needs the SDK path to link
  if [[ -z "${CXX:-}" ]] && command -v g++-15 > /dev/null; then
    export CXX=g++-15
    if [[ "$(uname)" == "Darwin" ]]; then export SDKROOT="${SDKROOT:-$(xcrun --show-sdk-path)}"; fi
  fi
  cmake --preset default || exit 1
  cmake --build --preset release || exit 1
  BUILD_DIR="out/build/default"
fi
build_dir=$(cd "${BUILD_DIR}" && pwd)

# Temporary folder where the programs run (they read input/input.txt and write their outputs)
work=$(mktemp -d)
trap 'rm -rf "${work}"' EXIT

passed=0
failed=0

pass() {
  echo "PASS  $1"
  passed=$((passed + 1))
}

fail() {
  echo "FAIL  $1"
  failed=$((failed + 1))
}

executable() {
  echo "${build_dir}/$1/Release/grav-$1"
}

# Creates an empty run folder for a test. If an input file is given, it is copied there.
prepare_run() {
  local dir=$1 input=$2
  rm -rf "${dir}"
  mkdir -p "${dir}/input"
  if [[ -n "${input}" ]]; then cp "${input}" "${dir}/input/input.txt"; fi
}

# Success case: runs both versions and compares their outputs with the expected files.
# Usage: check_success <case> <num_iterations> <width> <height> <time_step>
check_success() {
  local name=$1
  shift
  for version in aos soa; do
    local dir="${work}/${name}-${version}"
    prepare_run "${dir}" "${root}/ftest/${name}/input.txt"
    if ! (cd "${dir}" && "$(executable "${version}")" "$@" > stdout.txt 2> stderr.txt); then
      fail "${version} ${name}: exited with an error: $(cat "${dir}/stderr.txt")"
      continue
    fi
    local ok=1
    for output in init_config.txt final_config.txt; do
      if ! diff -u "${root}/ftest/${name}/${output}" "${dir}/${output}"; then
        fail "${version} ${name}: ${output} differs from the expected file"
        ok=0
      fi
    done
    if [[ ${ok} -eq 1 ]]; then pass "${version} ${name}"; fi
  done
}

# Error case: the program must exit with an error and print the expected message.
# Usage: check_error <description> <expected message> <input file or ""> <arguments...>
check_error() {
  local name=$1 message=$2 input=$3
  shift 3
  for version in aos soa; do
    local dir="${work}/error-${version}"
    prepare_run "${dir}" "${input}"
    if (cd "${dir}" && "$(executable "${version}")" "$@" > stdout.txt 2> stderr.txt); then
      fail "${version} ${name}: expected an error but the program succeeded"
    elif ! grep -qF -- "${message}" "${dir}/stderr.txt"; then
      fail "${version} ${name}: message should contain '${message}', got: $(cat "${dir}/stderr.txt")"
    else
      pass "${version} ${name}"
    fi
  done
}

# AOS vs SOA: both versions run on the same input and must write identical files.
# Usage: check_same_output <case> <num_iterations> <width> <height> <time_step>
check_same_output() {
  local name=$1
  shift
  for version in aos soa; do
    local dir="${work}/${name}-${version}"
    prepare_run "${dir}" "${root}/ftest/${name}/input.txt"
    if ! (cd "${dir}" && "$(executable "${version}")" "$@" > /dev/null 2>&1); then
      fail "${version} ${name}: exited with an error"
      return
    fi
  done
  for output in init_config.txt final_config.txt; do
    if cmp -s "${work}/${name}-aos/${output}" "${work}/${name}-soa/${output}"; then
      pass "aos and soa ${name}: same ${output}"
    else
      fail "aos and soa ${name}: ${output} differs"
    fi
  done
}

echo "=== Success cases ==="
check_success rebound 3 100 50 1
check_success invalid_input 2 100 50 0.5
check_success initial_collisions 1 100 50 1
check_success collision_in_simulation 2 100 50 1
check_success gravity 1 100 50 1
check_success no_valid_objects 5 100 50 1
# Official 1K use case: input and reference outputs provided by the instructor
check_success official_1k 500 1000 1000 0.25

echo "=== Error cases ==="
valid_input="${root}/ftest/rebound/input.txt"
check_error "no arguments" "invoked with 0 arguments" "${valid_input}"
check_error "too few arguments" "invoked with 3 arguments" "${valid_input}" 100 1000 500
check_error "too many arguments" "invoked with 5 arguments" "${valid_input}" 100 1000 500 0.25 7
check_error "zero iterations" "Invalid value for num_iterations: 0" "${valid_input}" 0 100 50 1
check_error "negative iterations" "Invalid value for num_iterations: -5" "${valid_input}" -5 100 50 1
check_error "non-numeric iterations" "Invalid value for 'num_iterations'" "${valid_input}" ten 100 50 1
check_error "negative width" "Invalid value for width: -1000" "${valid_input}" 100 -1000 2000 0.25
check_error "zero height" "Invalid value for height: 0" "${valid_input}" 100 1000 0 0.25
check_error "non-numeric time step" "Invalid value for 'time_step'" "${valid_input}" 100 1000 2000 abc
check_error "missing input file" "Cannot open input file" "" 10 100 50 1

echo "=== AOS vs SOA ==="
check_same_output aos_vs_soa 50 300 300 0.1

echo
echo "${passed} passed, ${failed} failed"
if [[ ${failed} -ne 0 ]]; then exit 1; fi
