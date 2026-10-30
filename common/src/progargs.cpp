#include "progargs.hpp"

#include <cmath>
#include <filesystem>
#include <format>
#include <ranges>
#include <stdexcept>

namespace common {

  namespace {

    int parse_positive_int(std::string const & name, std::string const & str) {
      int value = 0;
      try {
        std::size_t pos = 0;
        value           = std::stoi(str, &pos);
        if (pos != str.size()) { throw std::invalid_argument{"trailing characters"}; }
      } catch (std::exception const & e) {
        throw std::invalid_argument{std::format("Invalid value for '{}': '{}'", name, str)};
      }
      if (value <= 0) {
        throw std::invalid_argument{std::format("Invalid value for {}: {}", name, value)};
      }
      return value;
    }

    double parse_positive_double(std::string const & name, std::string const & str) {
      double value = 0.0;
      try {
        std::size_t pos = 0;
        value           = std::stod(str, &pos);
        if (pos != str.size()) { throw std::invalid_argument{"trailing characters"}; }
      } catch (std::exception const & e) {
        throw std::invalid_argument{std::format("Invalid value for '{}': '{}'", name, str)};
      }
      if (not std::isfinite(value) or value <= 0.0) {
        throw std::invalid_argument{std::format("Invalid value for {}: {}", name, value)};
      }
      return value;
    }

  }  // namespace

  program_arguments::program_arguments(std::vector<std::string> const & args) {
    if (args.empty()) { throw std::invalid_argument{"no arguments provided"}; }
    program_name_ = std::filesystem::path(args.at(0)).stem().string();
    if (args.size() != 5) {
      auto msg = std::format("{}: invoked with {} arguments", program_name_, args.size() - 1);
      msg += "\n\tExpected 4 arguments: <num_iterations> <width> <height> <time_step>";
      msg += std::format("\n\tReceived arguments: {}", args | std::views::drop(1));
      throw std::invalid_argument(msg);
    }
    num_iterations_ = parse_positive_int("num_iterations", args.at(1));
    width_          = parse_positive_double("width", args.at(2));
    height_         = parse_positive_double("height", args.at(3));
    time_step_      = parse_positive_double("time_step", args.at(4));
  }

}  // namespace common
