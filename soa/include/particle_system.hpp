#ifndef SOA_PARTICLE_SYSTEM_HPP
#define SOA_PARTICLE_SYSTEM_HPP

#include <enclosure.hpp>
#include <particle.hpp>
#include <point.hpp>

#include <cstddef>
#include <filesystem>
#include <ostream>
#include <vector>

namespace soa {

  /// Structure of arrays (SOA): every property of the objects is kept in its own vector.
  /// The object with index i is made of the i-th element of every vector, and the
  /// objects are ordered by increasing id.
  class particle_system {
  public:
    /// Creates the system from the objects read from the input file.
    particle_system(common::enclosure const & box, std::vector<common::particle> const & particles);

    /// Number of objects currently in the system.
    [[nodiscard]] std::size_t size() const noexcept { return mass_.size(); }

    /// Id, mass, radius, position and velocity of the object at the given index.
    [[nodiscard]] common::particle particle(std::size_t index) const;

    /// Acceleration of the object at the given index, computed in the last step.
    [[nodiscard]] common::point acceleration(std::size_t index) const;

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
    /// Appends an object at the end of every vector.
    void add(common::particle const & p);

    /// Replaces the properties of the object at the given index.
    void set(std::size_t index, common::particle const & p);

    /// Removes the object at the given index from every vector.
    void remove(std::size_t index);

    /// Position of the object at the given index.
    [[nodiscard]] common::point position(std::size_t index) const;

    /// Computes the acceleration of every object from the gravitational forces.
    void compute_accelerations();

    /// Updates velocities and positions, then applies the rebound on the walls.
    void move(double time_step);

    /// Finds the first colliding pair in id order and merges it.
    /// Returns false when no pair collides.
    bool merge_first_collision();

    common::enclosure box_;
    std::vector<std::size_t> id_;
    std::vector<double> mass_;
    std::vector<double> radius_;
    std::vector<double> pos_x_;
    std::vector<double> pos_y_;
    std::vector<double> vel_x_;
    std::vector<double> vel_y_;
    std::vector<double> acc_x_;
    std::vector<double> acc_y_;
  };

}  // namespace soa

#endif  // SOA_PARTICLE_SYSTEM_HPP
