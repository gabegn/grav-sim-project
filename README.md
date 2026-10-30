# Gravitational simulation (Computer Architecture, UC3M)

Sequential C++23 simulation of objects attracted by gravity in a 2D enclosure, with rebounds on the
walls and merging collisions. Two executables implement the same simulation with different data
layouts:

- `grav-aos`: array of structures, one `std::vector` of bodies
- `grav-soa`: structure of arrays, one `std::vector` per property

Both produce byte-identical output files, identical to the instructor's reference outputs for the
official 1K and 10K use cases.

## Build

Requires g++-15 and CMake 4.

```bash
CXX=g++-15 cmake --preset default && cmake --build --preset release
```

With static analysis (clang-tidy):

```bash
CXX=g++-15 cmake --preset clang-tidy && cmake --build --preset clang-tidy-release
```

## Run

```bash
out/build/default/aos/Release/grav-aos <num_iterations> <width> <height> <time_step>
```

The program reads `input/input.txt` in the current folder and writes `init_config.txt` and
`final_config.txt` there. `grav-soa` takes the same arguments.

## Tests

```bash
./utest.sh   # builds and runs the unit tests: utcommon, utaos, utsoa
./ftest.sh   # builds and runs the functional tests on both executables
```

`ftest/README.md` describes every functional test case.

## Project structure

| Folder | Contents |
|---|---|
| `common/` | Shared library: particle record and input parsing, physics, configuration input/output, program arguments |
| `aos/` | AOS version (`grav-aos`) |
| `soa/` | SOA version (`grav-soa`) |
| `utcommon/`, `utaos/`, `utsoa/` | Unit tests (GoogleTest) |
| `ftest/` | Functional test data |
| `evaluation/` | Performance and energy measurement scripts for the avignon cluster, results and figures |

## Performance and energy evaluation

`evaluation/run_all.sh` submits the measurements as SLURM jobs on an avignon node; each run is measured
with `perf stat` (time, package energy and core energy). `evaluation/plot.py` turns
`evaluation/results.csv` into the figures in `evaluation/figures/` and the tables in
`evaluation/summary.md`.
