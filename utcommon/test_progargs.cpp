#include "progargs.hpp"

#include <gtest/gtest.h>

TEST(test_progargs, zero_arguments) {
  std::vector<std::string> const args{};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, no_arguments) {
  std::vector<std::string> const args{"program_name"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_zero) {
  std::vector<std::string> const args{"program_name", "0", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_negative) {
  std::vector<std::string> const args{"program_name", "-10", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_non_numeric) {
  std::vector<std::string> const args{"program_name", "abc", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_iterations_partially_numeric) {
  std::vector<std::string> const args{"program_name", "10abc", "1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_zero) {
  std::vector<std::string> const args{"program_name", "10", "0.0", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_negative) {
  std::vector<std::string> const args{"program_name", "10", "-1000.5", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_non_numeric) {
  std::vector<std::string> const args{"program_name", "10", "abc", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_partially_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5abc", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_infinity) {
  std::vector<std::string> const args{"program_name", "10", "inf", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_width_nan) {
  std::vector<std::string> const args{"program_name", "10", "nan", "2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_zero) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "0.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_negative) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "-2000.0", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_non_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "abc", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_partially_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0abc", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_infinity) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "inf", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_height_nan) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "nan", "0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_zero) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "0.0"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_negative) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "-0.25"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_non_numeric) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "abc"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_infinity) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "inf"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, invalid_timestep_nan) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "nan"};
  EXPECT_THROW(common::program_arguments{args}, std::invalid_argument);
}

TEST(test_progargs, valid_arguments) {
  std::vector<std::string> const args{"program_name", "10", "1000.5", "2000.0", "0.25"};
  common::program_arguments const parsed_args{args};
  EXPECT_EQ(parsed_args.program_name(), "program_name");
  EXPECT_EQ(parsed_args.num_iterations(), 10);
  EXPECT_DOUBLE_EQ(parsed_args.width(), 1000.5);
  EXPECT_DOUBLE_EQ(parsed_args.height(), 2000.0);
  EXPECT_DOUBLE_EQ(parsed_args.time_step(), 0.25);
}
