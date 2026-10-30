#include "point.hpp"

#include <gtest/gtest.h>

#include <sstream>

TEST(test_point, default_constructor) {
  common::point const p;
  EXPECT_EQ(p.x, 0.0);
  EXPECT_EQ(p.y, 0.0);
}

TEST(test_point, extraction_operator) {
  std::istringstream iss("3.0 4.0");
  common::point p;
  iss >> p;
  EXPECT_EQ(p.x, 3.0);
  EXPECT_EQ(p.y, 4.0);
}

TEST(test_point, formatting) {
  common::point p{3.5, 4.5};
  std::string const expected = "3.5 4.5";
  std::string const actual   = std::format("{}", p);
  EXPECT_EQ(actual, expected);
}

TEST(test_point, addition) {
  common::point const sum = common::point{1.0, 2.0} + common::point{3.5, -4.0};
  EXPECT_EQ(sum.x, 4.5);
  EXPECT_EQ(sum.y, -2.0);
}

TEST(test_point, subtraction) {
  common::point const diff = common::point{1.0, 2.0} - common::point{3.5, -4.0};
  EXPECT_EQ(diff.x, -2.5);
  EXPECT_EQ(diff.y, 6.0);
}

TEST(test_point, negation) {
  common::point const neg = -common::point{1.5, -2.0};
  EXPECT_EQ(neg.x, -1.5);
  EXPECT_EQ(neg.y, 2.0);
}

TEST(test_point, scalar_multiplication) {
  common::point const scaled = common::point{1.5, -2.0} * 2.0;
  EXPECT_EQ(scaled.x, 3.0);
  EXPECT_EQ(scaled.y, -4.0);
}

TEST(test_point, scalar_multiplication_by_zero) {
  common::point const scaled = common::point{1.5, -2.0} * 0.0;
  EXPECT_EQ(scaled.x, 0.0);
  EXPECT_EQ(scaled.y, 0.0);
}

TEST(test_point, norm) {
  EXPECT_DOUBLE_EQ(common::norm(common::point{3.0, 4.0}), 5.0);
  EXPECT_DOUBLE_EQ(common::norm(common::point{-3.0, -4.0}), 5.0);
}

TEST(test_point, norm_of_zero_vector) {
  EXPECT_EQ(common::norm(common::point{}), 0.0);
}
