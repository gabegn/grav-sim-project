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
  
  // Continuously merges colliding particles until no overlaps remain
  void particle_system::resolve_collisions() {
    while (merge_first_collision()) { }
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
    for (auto & b : bodies_) { b.acceleration = {0.0, 0.0}; }
    for (auto first = bodies_.begin(); first != bodies_.end(); ++first) {
      for (auto second = std::next(first); second != bodies_.end(); ++second) {
        common::point const force = common::gravitational_force(
            first->state.mass, second->state.mass, second->state.position - first->state.position);
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
      b.state.position = common::update_position(b.state.position, b.state.velocity, time_step);
      common::rebound(b.state, box_);
    }
  }

  // Finds and resolves the first pair collision by merging into the dominant body
  bool particle_system::merge_first_collision() {
    for (auto first = bodies_.begin(); first != bodies_.end(); ++first) {
      for (auto second = std::next(first); second != bodies_.end(); ++second) {
        if (not common::collide(first->state.position, first->state.radius, second->state.position,
                                second->state.radius))
        {
          continue;
        }
        if (common::absorbs(common::key_of(first->state), common::key_of(second->state))) {
          first->state = common::merge(first->state, second->state);
          bodies_.erase(second);
        } else {
          second->state = common::merge(second->state, first->state);
          bodies_.erase(first);
        }
        return true;
      }
    }
    return false;
  }

}  // namespace aos