#include <cstdlib>
#include <exception>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include <config_io.hpp>
#include <enclosure.hpp>
#include <progargs.hpp>

#include "particle_system.hpp"

int main(int argc, char ** argv) try {
  std::span const spnargs{argv, static_cast<std::size_t>(argc)};
  std::vector<std::string> const vecargs{spnargs.begin(), spnargs.end()};
  common::program_arguments const progargs{vecargs};

  common::enclosure const box{.width = progargs.width(), .height = progargs.height()};
  aos::particle_system system{box, common::load_particles("input/input.txt", box)};

  system.resolve_collisions();
  system.save("init_config.txt", progargs.time_step());

  system.simulate(progargs.num_iterations(), progargs.time_step());
  system.save("final_config.txt", progargs.time_step());

  return EXIT_SUCCESS;

} catch (std::exception const & e) {
  std::cerr << "grav-aos: " << e.what() << '\n';
  return EXIT_FAILURE;

} catch (...) {
  std::cerr << "grav-aos: Unknown exception" << '\n';
  return EXIT_FAILURE;
}
