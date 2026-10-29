#include "config_io.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

  common::enclosure const box{.width = 100.0, .height = 50.0};

}  // namespace

TEST(test_config_io, read_particles_id_is_line_number) {
  std::istringstream input{"10 2 30 40 1 1\n"
                           "not a particle\n"
                           "-5 2 30 40 1 1\n"
                           "20 3 60 20 -1 0\n"};
  auto const particles = common::read_particles(input, box);
  ASSERT_EQ(particles.size(), 2U);
  EXPECT_EQ(particles.at(0).id, 0U);
  EXPECT_EQ(particles.at(0).mass, 10.0);
  EXPECT_EQ(particles.at(1).id, 3U);
  EXPECT_EQ(particles.at(1).mass, 20.0);
}

TEST(test_config_io, read_particles_empty_line_uses_an_id) {
  std::istringstream input{"\n10 2 30 40 1 1\n"};
  auto const particles = common::read_particles(input, box);
  ASSERT_EQ(particles.size(), 1U);
  EXPECT_EQ(particles.at(0).id, 1U);
}

TEST(test_config_io, read_particles_invalid_object_uses_an_id) {
  std::istringstream input{"10 2 1 40 1 1\n10 2 30 40 1 1\n"};
  auto const particles = common::read_particles(input, box);
  ASSERT_EQ(particles.size(), 1U);
  EXPECT_EQ(particles.at(0).id, 1U);
}

TEST(test_config_io, read_particles_skips_objects_outside_enclosure) {
  std::istringstream input{"10 2 1 40 1 1\n10 2 30 49 1 1\n"};
  EXPECT_TRUE(common::read_particles(input, box).empty());
}

TEST(test_config_io, read_particles_last_line_without_newline) {
  std::istringstream input{"10 2 30 40 1 1\n20 3 60 20 -1 0"};
  EXPECT_EQ(common::read_particles(input, box).size(), 2U);
}

TEST(test_config_io, read_particles_incomplete_last_line_is_ignored) {
  std::istringstream input{"10 2 30 40 1 1\n20 3 60"};
  EXPECT_EQ(common::read_particles(input, box).size(), 1U);
}

TEST(test_config_io, read_particles_empty_input) {
  std::istringstream input{""};
  EXPECT_TRUE(common::read_particles(input, box).empty());
}

TEST(test_config_io, load_particles_missing_file_throws) {
  auto const path = std::filesystem::temp_directory_path() / "grav-utcommon-does-not-exist.txt";
  std::filesystem::remove(path);
  EXPECT_THROW(static_cast<void>(common::load_particles(path, box)), std::runtime_error);
}

TEST(test_config_io, load_particles_reads_file) {
  auto const path = std::filesystem::temp_directory_path() / "grav-utcommon-input.txt";
  {
    std::ofstream output{path};
    output << "10 2 30 40 1 1\n20 3 60 20 -1 0\n";
  }
  auto const particles = common::load_particles(path, box);
  std::filesystem::remove(path);
  ASSERT_EQ(particles.size(), 2U);
  EXPECT_EQ(particles.at(1).id, 1U);
}

TEST(test_config_io, write_header) {
  std::ostringstream output;
  common::write_header(output, {.width = 1000.0, .height = 2000.0}, 0.25, 991);
  EXPECT_EQ(output.str(), "1000.000 2000.000 0.250 991\n");
}

TEST(test_config_io, write_particle) {
  std::ostringstream output;
  common::write_particle(output, {
                                   .id       = 3,
                                   .mass     = 3.4664,
                                   .radius   = 0.2449,
                                   .position = {726.2474, 179.0306},
                                   .velocity = {   8.828,  -2.7821},
  });
  EXPECT_EQ(output.str(), "3: 3.466 0.245 726.247 179.031 8.828 -2.782\n");
}

TEST(test_config_io, write_particle_rounds_to_three_decimals) {
  std::ostringstream output;
  common::write_particle(output, {
                                   .id       = 0,
                                   .mass     = 1.0,
                                   .radius   = 1.0,
                                   .position = {1.23456,     2.0},
                                   .velocity = {    0.0, -0.0004},
  });
  EXPECT_EQ(output.str(), "0: 1.000 1.000 1.235 2.000 0.000 -0.000\n");
}
