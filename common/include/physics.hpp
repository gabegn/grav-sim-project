#ifndef COMMON_PHYSICS_HPP
#define COMMON_PHYSICS_HPP

#include "enclosure.hpp"
#include "particle.hpp"
#include "point.hpp"

#include <cmath>
#include <cstddef>

// Physics helpers shared by the AOS and SOA versions. They are defined inline in this header
// because they are called from the innermost simulation loops, which live in other
// translation units: keeping them here lets the compiler inline them without LTO.

namespace common {

  /// Universal gravitational constant.
  inline constexpr double gravitational_constant = 6.674e-11;

  /// Force applied to object i by object j (equation 1), where delta = p_j - p_i.
  /// The force on j is the opposite vector. Requires a non-zero delta, which holds once
  /// collisions have been resolved since radii are positive.
  [[nodiscard]] inline point gravitational_force(double mass_i, double mass_j,
                                                 point const & delta) noexcept {
    double const dist   = norm(delta);
    double const factor = gravitational_constant * mass_i * mass_j;
    double const cube   = dist * dist * dist;
    return {.x = factor * delta.x / cube, .y = factor * delta.y / cube};
  }

  /// Acceleration from the sum of all forces applied to an object (equation 3).
  [[nodiscard]] constexpr point acceleration(point const & total_force, double mass) noexcept {
    return {.x = total_force.x / mass, .y = total_force.y / mass};
  }

  /// New velocity after one time step (equation 4).
  [[nodiscard]] constexpr point update_velocity(point const & velocity, point const & accel,
                                                double time_step) noexcept {
    return velocity + accel * time_step;
  }

  /// New position after one time step (equation 5).
  [[nodiscard]] constexpr point update_position(point const & position, point const & velocity,
                                                double time_step) noexcept {
    return position + velocity * time_step;
  }

  /// Rebound against the bounds [0, limit] along one axis: the object is placed on the
  /// bound it crossed and its velocity component is made to point to the interior.
  inline void rebound(double & coordinate, double & velocity, double radius,
                      double limit) noexcept {
    if (coordinate - radius <= 0.0) {
      coordinate = radius;
      velocity   = std::abs(velocity);
    } else if (coordinate + radius >= limit) {
      coordinate = limit - radius;
      velocity   = -std::abs(velocity);
    }
  }

  /// Applies the rebound rule on both axes of the enclosure.
  inline void rebound(particle & p, enclosure const & box) noexcept {
    rebound(p.position.x, p.velocity.x, p.radius, box.width);
    rebound(p.position.y, p.velocity.y, p.radius, box.height);
  }

  /// Two objects collide when their distance (equation 6) is less than the sum of radii.
  [[nodiscard]] inline bool collide(point const & position_i, double radius_i,
                                    point const & position_j, double radius_j) noexcept {
    return norm(position_i - position_j) < radius_i + radius_j;
  }

  /// Properties that decide which of two colliding objects absorbs the other.
  struct absorption_key {
    double mass    = 0.0;
    double radius  = 0.0;
    std::size_t id = 0;
  };

  /// Builds the absorption key of a particle.
  [[nodiscard]] constexpr absorption_key key_of(particle const & p) noexcept {
    return {.mass = p.mass, .radius = p.radius, .id = p.id};
  }

  /// True when first absorbs second: greater mass wins, then greater radius, then the
  /// smaller identifier.
  [[nodiscard]] constexpr bool absorbs(absorption_key const & first,
                                       absorption_key const & second) noexcept {
    if (first.mass != second.mass) { return first.mass > second.mass; }
    if (first.radius != second.radius) { return first.radius > second.radius; }
    return first.id < second.id;
  }

  /// Object resulting from absorber absorbing absorbed: masses and velocities are added,
  /// while id, radius and position are those of the absorber.
  [[nodiscard]] constexpr particle merge(particle const & absorber,
                                         particle const & absorbed) noexcept {
    return {
      .id       = absorber.id,
      .mass     = absorber.mass + absorbed.mass,
      .radius   = absorber.radius,
      .position = absorber.position,
      .velocity = absorber.velocity + absorbed.velocity,
    };
  }

}  // namespace common

#endif  // COMMON_PHYSICS_HPP
