#include "config_io.hpp"

#include <format>
#include <fstream>
#include <stdexcept>
#include <string>

namespace common {

  std::vector<particle> read_particles(std::istream & input, enclosure const & box) {
    std::vector<particle> particles;
    std::string line;
    // Every line consumes an id, even when it is malformed or the object is invalid
    // (instructor's answer on the course forum), so the id is the line number from 0
    for (std::size_t id = 0; std::getline(input, line); ++id) {
      auto parsed = parse_particle(line);
      if (parsed and is_valid(*parsed, box)) {
        parsed->id = id;
        particles.push_back(*parsed);
      }
    }
    return particles;
  }

  std::vector<particle> load_particles(std::filesystem::path const & path, enclosure const & box) {
    std::ifstream input{path};
    if (not input) {
      throw std::runtime_error{std::format("Cannot open input file: {}", path.string())};
    }
    return read_particles(input, box);
  }

  void write_header(std::ostream & output, enclosure const & box, double time_step,
                    std::size_t count) {
    output << std::format("{:.3f} {:.3f} {:.3f} {}\n", box.width, box.height, time_step, count);
  }

  void write_particle(std::ostream & output, particle const & p) {
    output << std::format("{}: {:.3f} {:.3f} {:.3f} {:.3f} {:.3f} {:.3f}\n", p.id, p.mass, p.radius,
                          p.position.x, p.position.y, p.velocity.x, p.velocity.y);
  }

}  // namespace common
