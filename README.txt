Project 1 AOS and SOA

Build with G++15 and CMake 4

  CXX=g++-15 cmake --preset default
  cmake --build --preset release

To run:

  Put the input file in input/input.txt, then:

  out/build/default/aos/Release/grav-aos <num_iterations> <width> <height> <time_step>
  out/build/default/soa/Release/grav-soa <num_iterations> <width> <height> <time_step>

  Example: out/build/default/aos/Release/grav-aos 500 1000 1000 0.25

  The output is written to init_config.txt and final_config.txt in the current folder.

Test:

  ./utest.sh    unit tests (builds the project first)
  ./ftest.sh    functional tests (builds the project first)
