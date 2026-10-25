#include "particle_system.hpp"

#include <config_io.hpp>
#include <physics.hpp>

#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace aos {

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

  void particle_system::resolve_collisions() {
    // After every merge the scan starts again from the lowest id, as the statement requires.
    while (merge_first_collision()) { }
  }

  void particle_system::step(double time_step) {
    compute_accelerations();
    move(time_step);
    resolve_collisions();
  }

  void particle_system::simulate(std::size_t num_iterations, double time_step) {
    for (std::size_t iteration = 0; iteration < num_iterations; ++iteration) { step(time_step); }
  }

  void particle_system::write(std::ostream & output, double time_step) const {
    common::write_header(output, box_, time_step, bodies_.size());
    for (auto const & b : bodies_) { common::write_particle(output, b.state); }
  }

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

  void particle_system::compute_accelerations() {
    // First add up the forces on every object, using the acceleration field to hold the sum.
    for (auto & b : bodies_) { b.acceleration = {0.0, 0.0}; }
    for (auto first = bodies_.begin(); first != bodies_.end(); ++first) {
      for (auto second = std::next(first); second != bodies_.end(); ++second) {
        common::point const force = common::gravitational_force(
            first->state.mass, second->state.mass, second->state.position - first->state.position);
        // Each pair is computed once: the second object gets the opposite force.
        first->acceleration  = first->acceleration + force;
        second->acceleration = second->acceleration - force;
      }
    }
    // Then divide each total force by the mass of the object (equation 3).
    for (auto & b : bodies_) {
      b.acceleration = common::acceleration(b.acceleration, b.state.mass);
    }
  }

  void particle_system::move(double time_step) {
    for (auto & b : bodies_) {
      b.state.velocity = common::update_velocity(b.state.velocity, b.acceleration, time_step);
      // Uses the velocity just updated (pending confirmation of equation 5).
      b.state.position = common::update_position(b.state.position, b.state.velocity, time_step);
      common::rebound(b.state, box_);
    }
  }

  bool particle_system::merge_first_collision() {
    for (auto first = bodies_.begin(); first != bodies_.end(); ++first) {
      for (auto second = std::next(first); second != bodies_.end(); ++second) {
        if (not common::collide(first->state.position, first->state.radius, second->state.position,
                                second->state.radius))
        {
          continue;
        }
        // The merged object replaces the absorber, so the vector stays ordered by id.
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
