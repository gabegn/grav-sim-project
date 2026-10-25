#include "particle_system.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

  common::enclosure const box{.width = 100.0, .height = 50.0};

  /// Object at rest with the given id, mass, radius and position.
  common::particle make_particle(std::size_t id, double mass, double radius, common::point pos) {
    return {
      .id       = id,
      .mass     = mass,
      .radius   = radius,
      .position = pos,
      .velocity = {0.0, 0.0},
    };
  }

  /// Object with the given position and velocity, a tiny mass and radius 1.
  common::particle make_moving(common::point pos, common::point vel) {
    return {
      .id       = 0,
      .mass     = 1.0,
      .radius   = 1.0,
      .position = pos,
      .velocity = vel,
    };
  }

}  // namespace

TEST(test_aos_particle_system, constructor_keeps_objects_in_order) {
  aos::particle_system const system{
    box, {make_particle(0, 1.0, 1.0, {10.0, 10.0}), make_particle(1, 2.0, 1.0, {50.0, 10.0})}
  };
  ASSERT_EQ(system.size(), 2U);
  EXPECT_EQ(system.bodies().at(0).state.id, 0U);
  EXPECT_EQ(system.bodies().at(1).state.id, 1U);
  EXPECT_EQ(system.bodies().at(1).state.mass, 2.0);
}

TEST(test_aos_particle_system, constructor_empty) {
  aos::particle_system const system{box, {}};
  EXPECT_EQ(system.size(), 0U);
}

TEST(test_aos_particle_system, no_collision_keeps_all_objects) {
  aos::particle_system system{
    box, {make_particle(0, 1.0, 1.0, {10.0, 10.0}), make_particle(1, 1.0, 1.0, {20.0, 10.0})}
  };
  system.resolve_collisions();
  EXPECT_EQ(system.size(), 2U);
}

TEST(test_aos_particle_system, collision_heavier_object_absorbs) {
  aos::particle_system system{
    box, {make_particle(0, 1.0, 3.0, {10.0, 10.0}), make_particle(1, 5.0, 2.0, {12.0, 10.0})}
  };
  system.resolve_collisions();
  ASSERT_EQ(system.size(), 1U);
  common::particle const & merged = system.bodies().at(0).state;
  EXPECT_EQ(merged.id, 1U);
  EXPECT_EQ(merged.mass, 6.0);
  EXPECT_EQ(merged.radius, 2.0);
  EXPECT_EQ(merged.position.x, 12.0);
}

TEST(test_aos_particle_system, collision_equal_objects_smaller_id_absorbs) {
  aos::particle_system system{
    box, {make_particle(0, 2.0, 2.0, {10.0, 10.0}), make_particle(1, 2.0, 2.0, {12.0, 10.0})}
  };
  system.resolve_collisions();
  ASSERT_EQ(system.size(), 1U);
  EXPECT_EQ(system.bodies().at(0).state.id, 0U);
  EXPECT_EQ(system.bodies().at(0).state.mass, 4.0);
}

TEST(test_aos_particle_system, collision_adds_velocities) {
  common::particle first  = make_particle(0, 3.0, 2.0, {10.0, 10.0});
  common::particle second = make_particle(1, 1.0, 2.0, {12.0, 10.0});
  first.velocity          = {1.0, 2.0};
  second.velocity         = {-4.0, 0.5};
  aos::particle_system system{
    box, {first, second}
  };
  system.resolve_collisions();
  ASSERT_EQ(system.size(), 1U);
  EXPECT_EQ(system.bodies().at(0).state.velocity.x, -3.0);
  EXPECT_EQ(system.bodies().at(0).state.velocity.y, 2.5);
}

TEST(test_aos_particle_system, several_collisions_merge_into_one_object) {
  std::vector<common::particle> const particles{
    make_particle(0, 1.0, 2.0, {10.0, 10.0}),
    make_particle(1, 5.0, 2.0, {12.0, 10.0}),
    make_particle(2, 2.0, 2.0, {14.0, 10.0}),
  };
  aos::particle_system system{box, particles};
  system.resolve_collisions();
  ASSERT_EQ(system.size(), 1U);
  EXPECT_EQ(system.bodies().at(0).state.id, 1U);
  EXPECT_EQ(system.bodies().at(0).state.mass, 8.0);
}

TEST(test_aos_particle_system, objects_stay_ordered_by_id_after_merge) {
  std::vector<common::particle> const particles{
    make_particle(0, 1.0, 1.0, {10.0, 10.0}),
    make_particle(1, 1.0, 1.0, {50.0, 10.0}),
    make_particle(2, 9.0, 1.0, {50.5, 10.0}),
    make_particle(3, 1.0, 1.0, {90.0, 10.0}),
  };
  aos::particle_system system{box, particles};
  system.resolve_collisions();
  ASSERT_EQ(system.size(), 3U);
  EXPECT_EQ(system.bodies().at(0).state.id, 0U);
  EXPECT_EQ(system.bodies().at(1).state.id, 2U);
  EXPECT_EQ(system.bodies().at(2).state.id, 3U);
}

TEST(test_aos_particle_system, step_moves_free_object_in_straight_line) {
  aos::particle_system system{box, {make_moving({10.0, 10.0}, {2.0, -1.0})}};
  system.step(0.5);
  common::particle const & moved = system.bodies().at(0).state;
  EXPECT_DOUBLE_EQ(moved.position.x, 11.0);
  EXPECT_DOUBLE_EQ(moved.position.y, 9.5);
  EXPECT_DOUBLE_EQ(moved.velocity.x, 2.0);
  EXPECT_DOUBLE_EQ(moved.velocity.y, -1.0);
}

TEST(test_aos_particle_system, step_objects_attract_each_other) {
  aos::particle_system system{
    box, {make_particle(0, 1e12, 1.0, {20.0, 25.0}), make_particle(1, 1e12, 1.0, {80.0, 25.0})}
  };
  system.step(1.0);
  auto const & bodies = system.bodies();
  EXPECT_GT(bodies.at(0).acceleration.x, 0.0);
  EXPECT_LT(bodies.at(1).acceleration.x, 0.0);
  EXPECT_DOUBLE_EQ(bodies.at(0).acceleration.x, -bodies.at(1).acceleration.x);
  EXPECT_GT(bodies.at(0).state.velocity.x, 0.0);
  EXPECT_LT(bodies.at(1).state.velocity.x, 0.0);
}

TEST(test_aos_particle_system, step_acceleration_value) {
  // a = G * m_other / d^2 = 6.674e-11 * 1e12 / 10^2 = 0.6674
  aos::particle_system system{
    box, {make_particle(0, 1.0, 1.0, {20.0, 25.0}), make_particle(1, 1e12, 1.0, {30.0, 25.0})}
  };
  system.step(1.0);
  EXPECT_NEAR(system.bodies().at(0).acceleration.x, 0.6674, 1e-12);
  EXPECT_DOUBLE_EQ(system.bodies().at(0).acceleration.y, 0.0);
}

TEST(test_aos_particle_system, step_rebounds_on_wall) {
  aos::particle_system system{box, {make_moving({98.0, 25.0}, {4.0, 0.0})}};
  system.step(1.0);
  common::particle const & moved = system.bodies().at(0).state;
  EXPECT_EQ(moved.position.x, 99.0);
  EXPECT_EQ(moved.velocity.x, -4.0);
}

TEST(test_aos_particle_system, step_resolves_collisions_at_the_end) {
  common::particle const first = make_moving({10.0, 25.0}, {5.0, 0.0});
  common::particle second      = make_moving({16.0, 25.0}, {0.0, 0.0});
  second.id                    = 1;
  aos::particle_system system{
    box, {first, second}
  };
  system.resolve_collisions();
  ASSERT_EQ(system.size(), 2U);
  system.step(1.0);
  EXPECT_EQ(system.size(), 1U);
}

TEST(test_aos_particle_system, simulate_runs_every_iteration) {
  aos::particle_system system{box, {make_moving({10.0, 25.0}, {1.0, 0.0})}};
  system.simulate(5, 2.0);
  EXPECT_DOUBLE_EQ(system.bodies().at(0).state.position.x, 20.0);
}

TEST(test_aos_particle_system, simulate_zero_iterations_changes_nothing) {
  aos::particle_system system{box, {make_moving({10.0, 25.0}, {1.0, 0.0})}};
  system.simulate(0, 2.0);
  EXPECT_EQ(system.bodies().at(0).state.position.x, 10.0);
}

TEST(test_aos_particle_system, write_configuration) {
  aos::particle_system const system{
    box, {make_particle(0, 1.0, 1.5, {10.0, 20.0}), make_particle(3, 2.0, 1.0, {50.0, 10.0})}
  };
  std::ostringstream output;
  system.write(output, 0.25);
  EXPECT_EQ(output.str(), "100.000 50.000 0.250 2\n"
                          "0: 1.000 1.500 10.000 20.000 0.000 0.000\n"
                          "3: 2.000 1.000 50.000 10.000 0.000 0.000\n");
}

TEST(test_aos_particle_system, save_writes_file) {
  auto const path = std::filesystem::temp_directory_path() / "grav-utaos-output.txt";
  aos::particle_system const system{box, {make_particle(0, 1.0, 1.5, {10.0, 20.0})}};
  system.save(path, 0.25);
  std::ifstream input{path};
  std::string header;
  std::getline(input, header);
  input.close();
  std::filesystem::remove(path);
  EXPECT_EQ(header, "100.000 50.000 0.250 1");
}

TEST(test_aos_particle_system, save_to_invalid_path_throws) {
  auto const path = std::filesystem::temp_directory_path() / "grav-no-such-dir" / "out.txt";
  aos::particle_system const system{box, {}};
  EXPECT_THROW(system.save(path, 0.25), std::runtime_error);
}
