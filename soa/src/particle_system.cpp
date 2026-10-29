#include "particle_system.hpp"

#include <config_io.hpp>
#include <physics.hpp>

#include <algorithm>
#include <cstddef>
#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace soa {

  namespace {

    // Removes the element at the given index from a vector, keeping the order of the rest
    template <typename T> void erase_at(std::vector<T> & values, std::size_t index) {
      values.erase(std::next(values.begin(), static_cast<std::ptrdiff_t>(index)));
    }

  }  // namespace

  // Constructor: copies every input particle into the property vectors
  particle_system::particle_system(common::enclosure const & box,
                                   std::vector<common::particle> const & particles)
      : box_{box} {
    for (auto const & p : particles) { add(p); }
  }

  // Builds a particle record from the i-th element of every vector
  common::particle particle_system::particle(std::size_t index) const {
    return {
      .id       = id_.at(index),
      .mass     = mass_.at(index),
      .radius   = radius_.at(index),
      .position = {pos_x_.at(index), pos_y_.at(index)},
      .velocity = {vel_x_.at(index), vel_y_.at(index)},
    };
  }

  // Returns the acceleration stored for the object at the given index
  common::point particle_system::acceleration(std::size_t index) const {
    return {acc_x_.at(index), acc_y_.at(index)};
  }

  // Merges colliding particles until no overlaps remain, checking every pair once in id order.
  // A merge never moves the surviving object or changes its radius, so pairs already checked
  // cannot start colliding. Continuing the scan therefore finds the same pairs, in the same
  // order, as restarting it from the lowest id after every merge (as the statement requires)
  void particle_system::resolve_collisions() {
    std::size_t i = 0;
    while (i < size()) {
      bool first_absorbed = false;
      std::size_t j       = i + 1;
      while (j < size() and not first_absorbed) {
        if (not common::collide(position(i), radius_.at(i), position(j), radius_.at(j))) {
          ++j;
        } else {
          // After the merge, index j (or i, if it was absorbed) holds the next object to check
          first_absorbed = merge_pair(i, j);
        }
      }
      if (not first_absorbed) { ++i; }
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
    common::write_header(output, box_, time_step, size());
    for (std::size_t i = 0; i < size(); ++i) { common::write_particle(output, particle(i)); }
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

  // Appends a new object with zero initial acceleration
  void particle_system::add(common::particle const & p) {
    id_.push_back(p.id);
    mass_.push_back(p.mass);
    radius_.push_back(p.radius);
    pos_x_.push_back(p.position.x);
    pos_y_.push_back(p.position.y);
    vel_x_.push_back(p.velocity.x);
    vel_y_.push_back(p.velocity.y);
    acc_x_.push_back(0.0);
    acc_y_.push_back(0.0);
  }

  // Overwrites id, mass, radius, position and velocity (the acceleration is left as it is)
  void particle_system::set(std::size_t index, common::particle const & p) {
    id_.at(index)     = p.id;
    mass_.at(index)   = p.mass;
    radius_.at(index) = p.radius;
    pos_x_.at(index)  = p.position.x;
    pos_y_.at(index)  = p.position.y;
    vel_x_.at(index)  = p.velocity.x;
    vel_y_.at(index)  = p.velocity.y;
  }

  // Removes the object from every vector, so all vectors keep the same length
  void particle_system::remove(std::size_t index) {
    erase_at(id_, index);
    erase_at(mass_, index);
    erase_at(radius_, index);
    erase_at(pos_x_, index);
    erase_at(pos_y_, index);
    erase_at(vel_x_, index);
    erase_at(vel_y_, index);
    erase_at(acc_x_, index);
    erase_at(acc_y_, index);
  }

  // Returns the position of the object at the given index as a point
  common::point particle_system::position(std::size_t index) const {
    return {pos_x_.at(index), pos_y_.at(index)};
  }

  // Computes gravitational forces between all pairs and updates accelerations (a = F / m)
  void particle_system::compute_accelerations() {
    // The acceleration vectors hold the sum of forces until the last loop divides it by the mass
    std::ranges::fill(acc_x_, 0.0);
    std::ranges::fill(acc_y_, 0.0);
    for (std::size_t i = 0; i < size(); ++i) {
      for (std::size_t j = i + 1; j < size(); ++j) {
        common::point const force =
            common::gravitational_force(mass_.at(i), mass_.at(j), position(j) - position(i));
        // Each pair is computed only once: the second object gets the opposite force (F_ji = -F_ij)
        acc_x_.at(i) += force.x;
        acc_y_.at(i) += force.y;
        acc_x_.at(j) -= force.x;
        acc_y_.at(j) -= force.y;
      }
    }
    for (std::size_t i = 0; i < size(); ++i) {
      acc_x_.at(i) /= mass_.at(i);
      acc_y_.at(i) /= mass_.at(i);
    }
  }

  // Updates velocity and position for each particle, then handles boundary bounces
  void particle_system::move(double time_step) {
    for (std::size_t i = 0; i < size(); ++i) {
      vel_x_.at(i) += acc_x_.at(i) * time_step;
      vel_y_.at(i) += acc_y_.at(i) * time_step;
      // Equation 5 uses the velocity just updated by equation 4 (steps are applied in order)
      pos_x_.at(i) += vel_x_.at(i) * time_step;
      pos_y_.at(i) += vel_y_.at(i) * time_step;
      common::rebound(pos_x_.at(i), vel_x_.at(i), radius_.at(i), box_.width);
      common::rebound(pos_y_.at(i), vel_y_.at(i), radius_.at(i), box_.height);
    }
  }

  // Merges the colliding objects i and j (i < j) into the absorber (larger mass, then larger
  // radius, then smaller id) and removes the other one. Returns true when object i was absorbed
  bool particle_system::merge_pair(std::size_t i, std::size_t j) {
    common::particle const first  = particle(i);
    common::particle const second = particle(j);
    // The merged object takes the absorber's place, so the vectors stay ordered by id
    if (common::absorbs(common::key_of(first), common::key_of(second))) {
      set(i, common::merge(first, second));
      remove(j);
      return false;
    }
    set(j, common::merge(second, first));
    remove(i);
    return true;
  }

}  // namespace soa
