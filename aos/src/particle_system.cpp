#include "particle_system.hpp"

#include <config_io.hpp>
#include <physics.hpp>

#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace aos {

  // Constructor: copies input particles into internal bodies with zero initial acceleration
  particle_system::particle_system(common::enclosure const & box,
                                   std::vector<common::particle> const & particles)
      : box_{box} {
    bodies_.reserve(particles.size());
    for (auto const & p : particles) {
      body const b{
        .state        = p,
        .acceleration = {0.0, 0.0},
      };
      bodies_.push_back(b);
    }
  }

  // Merges colliding particles until no overlaps remain, checking every pair once in id order.
  // A merge never moves the surviving object or changes its radius, so pairs already checked
  // cannot start colliding. Continuing the scan therefore finds the same pairs, in the same
  // order, as restarting it from the lowest id after every merge (as the statement requires).
  // The merged object takes the absorber's place, so the vector stays ordered by id
  void particle_system::resolve_collisions() {
    auto first = bodies_.begin();
    while (first != bodies_.end()) {
      bool first_absorbed = false;
      auto second         = std::next(first);
      while (second != bodies_.end() and not first_absorbed) {
        if (not common::collide(first->state.position, first->state.radius, second->state.position,
                                second->state.radius))
        {
          ++second;
        } else if (common::absorbs(common::key_of(first->state), common::key_of(second->state))) {
          // first absorbs second: keep checking first against the objects after second
          first->state = common::merge(first->state, second->state);
          second       = bodies_.erase(second);
        } else {
          // second absorbs first: first disappears and the scan goes on with the next object
          second->state  = common::merge(second->state, first->state);
          first          = bodies_.erase(first);
          first_absorbed = true;
        }
      }
      if (not first_absorbed) { ++first; }
    }
  }

  // Advances the system by one time step: forces -> movement -> collisions
  void particle_system::step(double time_step) {
    compute_accelerations();
    move(time_step);
    resolve_collisions();
  }

  // Runs the simulation for a given number of iterations
  void particle_system::simulate(std::size_t num_iterations, double time_step) {
    for (std::size_t iteration = 0; iteration < num_iterations; ++iteration) { step(time_step); }
  }

  // Writes the enclosure header and all particle states to a stream
  void particle_system::write(std::ostream & output, double time_step) const {
    common::write_header(output, box_, time_step, bodies_.size());
    for (auto const & b : bodies_) { common::write_particle(output, b.state); }
  }

  // Opens a file and writes the current system configuration to disk
  void particle_system::save(std::filesystem::path const & path, double time_step) const {
    std::ofstream output{path};
    if (not output) {
      throw std::runtime_error{std::format("Cannot open output file: {}", path.string())};
    }
    write(output, time_step);
    if (not output) {
      throw std::runtime_error{std::format("Error writing output file: {}", path.string())};
    }
  }

  // Computes gravitational forces between all pairs and updates accelerations (a = F / m)
  void particle_system::compute_accelerations() {
    // The acceleration field holds the sum of forces until the last loop divides it by the mass
    for (auto & b : bodies_) { b.acceleration = {0.0, 0.0}; }
    for (auto first = bodies_.begin(); first != bodies_.end(); ++first) {
      for (auto second = std::next(first); second != bodies_.end(); ++second) {
        common::point const force = common::gravitational_force(
            first->state.mass, second->state.mass, second->state.position - first->state.position);
        // Each pair is computed only once: the second object gets the opposite force (F_ji = -F_ij)
        first->acceleration  = first->acceleration + force;
        second->acceleration = second->acceleration - force;
      }
    }
    for (auto & b : bodies_) {
      b.acceleration = common::acceleration(b.acceleration, b.state.mass);
    }
  }

  // Updates velocity and position for each particle, then handles boundary bounces
  void particle_system::move(double time_step) {
    for (auto & b : bodies_) {
      b.state.velocity = common::update_velocity(b.state.velocity, b.acceleration, time_step);
      // Equation 5 uses the velocity just updated by equation 4 (steps are applied in order)
      b.state.position = common::update_position(b.state.position, b.state.velocity, time_step);
      common::rebound(b.state, box_);
    }
  }

}  // namespace aos
