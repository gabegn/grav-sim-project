#ifndef AOS_PARTICLE_SYSTEM_HPP
#define AOS_PARTICLE_SYSTEM_HPP

#include <enclosure.hpp>
#include <particle.hpp>
#include <point.hpp>

#include <cstddef>
#include <filesystem>
#include <ostream>
#include <vector>

namespace aos {

  /// One simulated object with all of its properties stored together.
  struct body {
    common::particle state;      ///< id, mass, radius, position and velocity
    common::point acceleration;  ///< acceleration computed in the last step
  };

  /// Array of structures (AOS): the objects are kept in a single vector of bodies,
  /// ordered by increasing id.
  class particle_system {
  public:
    /// Creates the system from the objects read from the input file.
    particle_system(common::enclosure const & box, std::vector<common::particle> const & particles);

    /// Number of objects currently in the system.
    [[nodiscard]] std::size_t size() const noexcept { return bodies_.size(); }

    /// Read-only access to the objects (used by the unit tests).
    [[nodiscard]] std::vector<body> const & bodies() const noexcept { return bodies_; }

    /// Merges colliding objects until no pair of objects collides.
    void resolve_collisions();

    /// Runs one iteration: accelerations, velocities, positions, rebounds and collisions.
    void step(double time_step);

    /// Runs the given number of iterations.
    void simulate(std::size_t num_iterations, double time_step);

    /// Writes the configuration (header plus one line per object).
    void write(std::ostream & output, double time_step) const;

    /// Writes the configuration to a file. Throws std::runtime_error on failure.
    void save(std::filesystem::path const & path, double time_step) const;

  private:
    /// Computes the acceleration of every object from the gravitational forces.
    void compute_accelerations();

    /// Updates velocities and positions, then applies the rebound on the walls.
    void move(double time_step);
    common::enclosure box_;
    std::vector<body> bodies_;
  };

}  // namespace aos

#endif  // AOS_PARTICLE_SYSTEM_HPP
