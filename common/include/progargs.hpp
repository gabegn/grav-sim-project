#ifndef COMMON_PROGARGS_HPP
#define COMMON_PROGARGS_HPP

#include <string>
#include <vector>

namespace common {

  class program_arguments {
  public:
    explicit program_arguments(std::vector<std::string> const & args);

    [[nodiscard]] std::string const & program_name() const { return program_name_; }

    [[nodiscard]] std::size_t num_iterations() const { return num_iterations_; }

    [[nodiscard]] double width() const { return width_; }

    [[nodiscard]] double height() const { return height_; }

    [[nodiscard]] double time_step() const { return time_step_; }

  private:
    std::string program_name_;
    std::size_t num_iterations_;
    double width_;
    double height_;
    double time_step_;
  };

}  // namespace common

#endif  // COMMON_PROGARGS_HPP
