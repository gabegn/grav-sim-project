#ifndef COMMON_CONFIG_IO_HPP
#define COMMON_CONFIG_IO_HPP

#include "enclosure.hpp"
#include "particle.hpp"

#include <cstddef>
#include <filesystem>
#include <istream>
#include <ostream>
#include <vector>

namespace common {

  /// Reads objects until end of input, skipping non-conforming lines and invalid objects.
  /// Each object's id is its line number starting at 0: skipped lines still use up an id.
  [[nodiscard]] std::vector<particle> read_particles(std::istream & input, enclosure const & box);

  /// Same as read_particles on the file at path. Throws std::runtime_error if it cannot be
  /// opened.
  [[nodiscard]] std::vector<particle> load_particles(std::filesystem::path const & path,
                                                     enclosure const & box);

  /// Writes the configuration header: width, height, time step and number of objects.
  void write_header(std::ostream & output, enclosure const & box, double time_step,
                    std::size_t count);

  /// Writes one object line: "id: mass radius x y vx vy".
  void write_particle(std::ostream & output, particle const & p);

}  // namespace common

#endif  // COMMON_CONFIG_IO_HPP
