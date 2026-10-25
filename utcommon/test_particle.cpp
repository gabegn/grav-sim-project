#include "particle.hpp"

#include <gtest/gtest.h>

#include <string_view>

namespace {

  common::particle make_particle(double mass, double radius, double x, double y) {
    return {
      .id       = 0,
      .mass     = mass,
      .radius   = radius,
      .position = {  x,   y},
      .velocity = {0.0, 0.0},
    };
  }

  common::enclosure const box{.width = 100.0, .height = 50.0};

  /// Parses a line that is expected to conform, reporting a test failure otherwise.
  common::particle parse_conforming(std::string_view line) {
    auto const parsed = common::parse_particle(line);
    if (not parsed) {
      ADD_FAILURE() << "line should conform: " << line;
      return {};
    }
    return *parsed;
  }

}  // namespace

TEST(test_particle, parse_valid_line) {
  auto const p = parse_conforming("10.5 2.0 30.0 40.0 -1.5 0.25");
  EXPECT_EQ(p.id, 0U);
  EXPECT_EQ(p.mass, 10.5);
  EXPECT_EQ(p.radius, 2.0);
  EXPECT_EQ(p.position.x, 30.0);
  EXPECT_EQ(p.position.y, 40.0);
  EXPECT_EQ(p.velocity.x, -1.5);
  EXPECT_EQ(p.velocity.y, 0.25);
}

TEST(test_particle, parse_extra_blanks_and_tabs) {
  auto const p = parse_conforming("  10\t2   30 40\t\t-1 1  ");
  EXPECT_EQ(p.mass, 10.0);
  EXPECT_EQ(p.velocity.y, 1.0);
}

TEST(test_particle, parse_windows_line_ending) {
  auto const p = parse_conforming("10 2 30 40 -1 1\r");
  EXPECT_EQ(p.velocity.y, 1.0);
}

TEST(test_particle, parse_scientific_notation) {
  auto const p = parse_conforming("1e3 2.5E-1 3e1 4e1 0 0");
  EXPECT_EQ(p.mass, 1000.0);
  EXPECT_EQ(p.radius, 0.25);
}

TEST(test_particle, parse_empty_line) {
  EXPECT_FALSE(common::parse_particle("").has_value());
  EXPECT_FALSE(common::parse_particle("   \t ").has_value());
}

TEST(test_particle, parse_too_few_fields) {
  EXPECT_FALSE(common::parse_particle("10 2 30 40 -1").has_value());
}

TEST(test_particle, parse_too_many_fields) {
  EXPECT_FALSE(common::parse_particle("10 2 30 40 -1 1 7").has_value());
}

TEST(test_particle, parse_non_numeric_field) {
  EXPECT_FALSE(common::parse_particle("10 two 30 40 -1 1").has_value());
}

TEST(test_particle, parse_trailing_characters_in_field) {
  EXPECT_FALSE(common::parse_particle("10 2 30x 40 -1 1").has_value());
}

TEST(test_particle, parse_non_finite_field) {
  EXPECT_FALSE(common::parse_particle("inf 2 30 40 -1 1").has_value());
  EXPECT_FALSE(common::parse_particle("10 2 30 nan -1 1").has_value());
}

TEST(test_particle, valid_inside_enclosure) {
  EXPECT_TRUE(common::is_valid(make_particle(1.0, 2.0, 50.0, 25.0), box));
}

TEST(test_particle, valid_touching_bounds) {
  EXPECT_TRUE(common::is_valid(make_particle(1.0, 2.0, 2.0, 2.0), box));
  EXPECT_TRUE(common::is_valid(make_particle(1.0, 2.0, 98.0, 48.0), box));
}

TEST(test_particle, invalid_mass) {
  EXPECT_FALSE(common::is_valid(make_particle(0.0, 2.0, 50.0, 25.0), box));
  EXPECT_FALSE(common::is_valid(make_particle(-1.0, 2.0, 50.0, 25.0), box));
}

TEST(test_particle, invalid_radius) {
  EXPECT_FALSE(common::is_valid(make_particle(1.0, 0.0, 50.0, 25.0), box));
  EXPECT_FALSE(common::is_valid(make_particle(1.0, -2.0, 50.0, 25.0), box));
}

TEST(test_particle, invalid_x_outside) {
  EXPECT_FALSE(common::is_valid(make_particle(1.0, 2.0, 1.999, 25.0), box));
  EXPECT_FALSE(common::is_valid(make_particle(1.0, 2.0, 98.001, 25.0), box));
}

TEST(test_particle, invalid_y_outside) {
  EXPECT_FALSE(common::is_valid(make_particle(1.0, 2.0, 50.0, 1.999), box));
  EXPECT_FALSE(common::is_valid(make_particle(1.0, 2.0, 50.0, 48.001), box));
}

TEST(test_particle, invalid_object_larger_than_enclosure) {
  EXPECT_FALSE(common::is_valid(make_particle(1.0, 30.0, 50.0, 25.0), box));
}
