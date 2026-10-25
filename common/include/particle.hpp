#ifndef COMMON_PARTICLE_HPP
#define COMMON_PARTICLE_HPP

#include "enclosure.hpp"
#include "point.hpp"

#include <cstddef>
#include <optional>
#include <string_view>

namespace common {

  /// Properties of one object as read from or written to a configuration file.
  /// Both the AOS and SOA versions are built from and written back as this record.
  struct particle {
    std::size_t id = 0;
    double mass    = 0.0;
    double radius  = 0.0;
    point position;
    point velocity;
  };

  /// Parses a line with six blank-separated finite numbers: mass, radius, x, y, vx, vy.
  /// Returns no value when the line does not conform to that format. The id is left as 0.
  [[nodiscard]] std::optional<particle> parse_particle(std::string_view line);

  /// Checks the initial configuration rules: positive mass and radius, and the whole
  /// object inside the enclosure (r <= x <= width - r and r <= y <= height - r).
  [[nodiscard]] bool is_valid(particle const & p, enclosure const & box) noexcept;

}  // namespace common

#endif  // COMMON_PARTICLE_HPP
