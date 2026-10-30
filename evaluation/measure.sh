#!/usr/bin/env bash
# Measures execution time and energy of grav-aos and grav-soa with perf, on the machine it
# runs on. Run it on an avignon compute node through SLURM: sbatch evaluation/measure.sbatch
#
# Experiments (box 1000 x 1000, time step 0.25, each run repeated 5 times):
#  - official:   the two official use cases, 1K x 500 iterations and 10K x 100 iterations
#  - objects:    100 iterations with 1000, 2000, 4000, 6000 and 8000 objects (the first N
#                lines of input10K.txt); the 10000-object point is the official 10K case
#  - iterations: the official 1K input with 125, 250, 500, 1000 and 2000 iterations
#
# Results are appended to evaluation/results.csv (created with a header if missing),
# one line per run:
#   experiment,version,objects,iterations,repetition,seconds,energy_pkg_j,energy_cores_j
#
# Usage: evaluation/measure.sh [aos|soa|all] [official|objects|iterations|all]...
#   Default: all versions and all experiments. Student jobs are limited to 10 minutes,
#   so measure one version and a few experiments per job, for example:
#     sbatch --nodelist=avignon1 --time=00:10:00 evaluation/measure.sbatch soa objects
# The path to input10K.txt can be changed with INPUT10K (default: ~/inputs/input10K.txt).
set -euo pipefail
export LC_ALL=C  # decimal points instead of commas in perf output

cd "$(dirname "$0")/.."
root=$(pwd)
input1k="${root}/ftest/official_1k/input.txt"
input10k="${INPUT10K:-${HOME}/inputs/input10K.txt}"
build_dir="${root}/out/build/default"
results="${root}/evaluation/results.csv"
repetitions=5

versions="${1:-all}"
if [[ "${versions}" == "all" ]]; then versions="aos soa"; fi
shift || true
experiments="${*:-all}"
if [[ "${experiments}" == "all" ]]; then experiments="official objects iterations"; fi

for file in "${input1k}" "${input10k}"; do
  if [[ ! -f "${file}" ]]; then
    echo "Input file not found: ${file}" >&2
    exit 1
  fi
done
for version in ${versions}; do
  if [[ ! -x "${build_dir}/${version}/Release/grav-${version}" ]]; then
    echo "Build the project first: CXX=g++-15 cmake --preset default && cmake --build --preset release" >&2
    exit 1
  fi
done

work=$(mktemp -d)
trap 'rm -rf "${work}"' EXIT

echo "Measuring on $(hostname), $(lscpu | grep 'Model name' | sed 's/.*: *//')"
echo "Versions: ${versions}; experiments: ${experiments}"
if [[ ! -f "${results}" ]]; then
  echo "experiment,version,objects,iterations,repetition,seconds,energy_pkg_j,energy_cores_j" \
    > "${results}"
fi

# Runs one configuration several times and appends one CSV line per run.
# Usage: measure <experiment> <version> <input file> <objects> <iterations>
measure() {
  local experiment=$1 version=$2 input=$3 objects=$4 iterations=$5
  local run="${work}/run"
  rm -rf "${run}"
  mkdir -p "${run}/input"
  head -n "${objects}" "${input}" > "${run}/input/input.txt"
  for repetition in $(seq 1 "${repetitions}"); do
    # perf -x, writes one "value,unit,event,..." line per event to perf.csv.
    # Energy counters cover the whole processor, so they need -a (system wide)
    (cd "${run}" && perf stat -x, -a -o "${work}/perf.csv" \
      -e duration_time,power/energy-pkg/,power/energy-cores/ \
      "${build_dir}/${version}/Release/grav-${version}" "${iterations}" 1000 1000 0.25 > /dev/null)
    local seconds pkg cores
    seconds=$(awk -F, '$3 == "duration_time" { printf "%.6f", $1 / 1e9 }' "${work}/perf.csv")
    pkg=$(awk -F, '$3 == "power/energy-pkg/" { print $1 }' "${work}/perf.csv")
    cores=$(awk -F, '$3 == "power/energy-cores/" { print $1 }' "${work}/perf.csv")
    echo "${experiment},${version},${objects},${iterations},${repetition},${seconds},${pkg},${cores}" \
      | tee -a "${results}"
  done
}

for version in ${versions}; do
  for experiment in ${experiments}; do
    case "${experiment}" in
      official)
        measure official "${version}" "${input1k}" 1000 500
        measure official "${version}" "${input10k}" 10000 100
        ;;
      objects)
        for objects in 1000 2000 4000 6000 8000; do
          measure objects "${version}" "${input10k}" "${objects}" 100
        done
        ;;
      iterations)
        for iterations in 125 250 500 1000 2000; do
          measure iterations "${version}" "${input1k}" 1000 "${iterations}"
        done
        ;;
      *)
        echo "Unknown experiment: ${experiment} (use official, objects or iterations)" >&2
        exit 1
        ;;
    esac
  done
done

echo "Results appended to ${results}"
