#include "physics.hpp"

#include <gtest/gtest.h>

namespace {

  common::enclosure const box{.width = 100.0, .height = 50.0};

  common::particle make_particle(common::point position, common::point velocity) {
    return {.id = 0, .mass = 1.0, .radius = 2.0, .position = position, .velocity = velocity};
  }

}  // namespace

TEST(test_physics, gravitational_force_value) {
  // |F| = G * 1e10 * 1e10 / 5^2 = 2.6696e8, directed along (3, 4) / 5.
  common::point const force = common::gravitational_force(1e10, 1e10, {3.0, 4.0});
  EXPECT_NEAR(force.x, 1.60176e8, 1e-6 * 1.60176e8);
  EXPECT_NEAR(force.y, 2.13568e8, 1e-6 * 2.13568e8);
}

TEST(test_physics, gravitational_force_points_towards_other_object) {
  common::point const force = common::gravitational_force(5.0, 7.0, {-2.0, 0.0});
  EXPECT_LT(force.x, 0.0);
  EXPECT_EQ(force.y, 0.0);
}

TEST(test_physics, gravitational_force_is_antisymmetric) {
  common::point const delta{3.0, -1.5};
  common::point const f_ij = common::gravitational_force(2.0, 9.0, delta);
  common::point const f_ji = common::gravitational_force(9.0, 2.0, -delta);
  EXPECT_DOUBLE_EQ(f_ij.x, -f_ji.x);
  EXPECT_DOUBLE_EQ(f_ij.y, -f_ji.y);
}

TEST(test_physics, acceleration) {
  common::point const accel = common::acceleration({10.0, -4.0}, 2.0);
  EXPECT_EQ(accel.x, 5.0);
  EXPECT_EQ(accel.y, -2.0);
}

TEST(test_physics, update_velocity) {
  common::point const v = common::update_velocity({1.0, 2.0}, {4.0, -8.0}, 0.25);
  EXPECT_EQ(v.x, 2.0);
  EXPECT_EQ(v.y, 0.0);
}

TEST(test_physics, update_position) {
  common::point const p = common::update_position({10.0, 20.0}, {-4.0, 8.0}, 0.5);
  EXPECT_EQ(p.x, 8.0);
  EXPECT_EQ(p.y, 24.0);
}

TEST(test_physics, rebound_inside_is_unchanged) {
  double coordinate = 50.0;
  double velocity   = -3.0;
  common::rebound(coordinate, velocity, 2.0, 100.0);
  EXPECT_EQ(coordinate, 50.0);
  EXPECT_EQ(velocity, -3.0);
}

TEST(test_physics, rebound_lower_bound) {
  double coordinate = 1.0;
  double velocity   = -3.0;
  common::rebound(coordinate, velocity, 2.0, 100.0);
  EXPECT_EQ(coordinate, 2.0);
  EXPECT_EQ(velocity, 3.0);
}

TEST(test_physics, rebound_lower_bound_already_moving_inwards) {
  double coordinate = -1.0;
  double velocity   = 3.0;
  common::rebound(coordinate, velocity, 2.0, 100.0);
  EXPECT_EQ(coordinate, 2.0);
  EXPECT_EQ(velocity, 3.0);
}

TEST(test_physics, rebound_upper_bound) {
  double coordinate = 99.0;
  double velocity   = 3.0;
  common::rebound(coordinate, velocity, 2.0, 100.0);
  EXPECT_EQ(coordinate, 98.0);
  EXPECT_EQ(velocity, -3.0);
}

TEST(test_physics, rebound_touching_bound_counts) {
  double coordinate = 98.0;
  double velocity   = 3.0;
  common::rebound(coordinate, velocity, 2.0, 100.0);
  EXPECT_EQ(coordinate, 98.0);
  EXPECT_EQ(velocity, -3.0);
}

TEST(test_physics, rebound_particle_both_axes) {
  common::particle p = make_particle({-5.0, 60.0}, {-1.0, 2.0});
  common::rebound(p, box);
  EXPECT_EQ(p.position.x, 2.0);
  EXPECT_EQ(p.position.y, 48.0);
  EXPECT_EQ(p.velocity.x, 1.0);
  EXPECT_EQ(p.velocity.y, -2.0);
}

TEST(test_physics, rebound_particle_inside_is_unchanged) {
  common::particle p = make_particle({50.0, 25.0}, {-1.0, 2.0});
  common::rebound(p, box);
  EXPECT_EQ(p.position.x, 50.0);
  EXPECT_EQ(p.position.y, 25.0);
  EXPECT_EQ(p.velocity.x, -1.0);
  EXPECT_EQ(p.velocity.y, 2.0);
}

TEST(test_physics, collide_overlapping) {
  EXPECT_TRUE(common::collide({0.0, 0.0}, 2.0, {3.0, 4.0}, 3.5));
}

TEST(test_physics, collide_apart) {
  EXPECT_FALSE(common::collide({0.0, 0.0}, 2.0, {3.0, 4.0}, 2.5));
}

TEST(test_physics, collide_touching_is_not_a_collision) {
  EXPECT_FALSE(common::collide({0.0, 0.0}, 2.0, {3.0, 4.0}, 3.0));
}

TEST(test_physics, absorbs_greater_mass) {
  EXPECT_TRUE(common::absorbs({.mass = 2.0, .radius = 1.0, .id = 5},
                              {.mass = 1.0, .radius = 9.0, .id = 0}));
  EXPECT_FALSE(common::absorbs({.mass = 1.0, .radius = 9.0, .id = 0},
                               {.mass = 2.0, .radius = 1.0, .id = 5}));
}

TEST(test_physics, absorbs_equal_mass_greater_radius) {
  EXPECT_TRUE(common::absorbs({.mass = 2.0, .radius = 3.0, .id = 5},
                              {.mass = 2.0, .radius = 1.0, .id = 0}));
  EXPECT_FALSE(common::absorbs({.mass = 2.0, .radius = 1.0, .id = 0},
                               {.mass = 2.0, .radius = 3.0, .id = 5}));
}

TEST(test_physics, absorbs_equal_mass_and_radius_smaller_id) {
  EXPECT_TRUE(common::absorbs({.mass = 2.0, .radius = 3.0, .id = 1},
                              {.mass = 2.0, .radius = 3.0, .id = 4}));
  EXPECT_FALSE(common::absorbs({.mass = 2.0, .radius = 3.0, .id = 4},
                               {.mass = 2.0, .radius = 3.0, .id = 1}));
}

TEST(test_physics, key_of) {
  common::particle const p{.id = 7, .mass = 3.0, .radius = 1.5, .position = {}, .velocity = {}};
  common::absorption_key const key = common::key_of(p);
  EXPECT_EQ(key.mass, 3.0);
  EXPECT_EQ(key.radius, 1.5);
  EXPECT_EQ(key.id, 7U);
}

TEST(test_physics, merge) {
  common::particle const absorber{
    .id = 3, .mass = 5.0, .radius = 2.0, .position = {10.0, 20.0}, .velocity = {1.0, -1.0}};
  common::particle const absorbed{
    .id = 8, .mass = 1.5, .radius = 4.0, .position = {11.0, 21.0}, .velocity = {0.5, 2.0}};
  common::particle const merged = common::merge(absorber, absorbed);
  EXPECT_EQ(merged.id, 3U);
  EXPECT_EQ(merged.mass, 6.5);
  EXPECT_EQ(merged.radius, 2.0);
  EXPECT_EQ(merged.position.x, 10.0);
  EXPECT_EQ(merged.position.y, 20.0);
  EXPECT_EQ(merged.velocity.x, 1.5);
  EXPECT_EQ(merged.velocity.y, 1.0);
}
