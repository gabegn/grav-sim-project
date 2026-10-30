#ifndef COMMON_POINT_HPP
#define COMMON_POINT_HPP

#include <cmath>
#include <format>
#include <istream>
#include <string_view>

namespace common {

  /// Two-dimensional vector used for positions, velocities, accelerations and forces.
  class point {
  public:
    double x = 0.0;
    double y = 0.0;
  };

  /// Component-wise addition.
  [[nodiscard]] constexpr point operator+(point const & lhs, point const & rhs) noexcept {
    return {.x = lhs.x + rhs.x, .y = lhs.y + rhs.y};
  }

  /// Component-wise subtraction.
  [[nodiscard]] constexpr point operator-(point const & lhs, point const & rhs) noexcept {
    return {.x = lhs.x - rhs.x, .y = lhs.y - rhs.y};
  }

  /// Opposite vector.
  [[nodiscard]] constexpr point operator-(point const & p) noexcept {
    return {.x = -p.x, .y = -p.y};
  }

  /// Multiplication by a scalar.
  [[nodiscard]] constexpr point operator*(point const & p, double factor) noexcept {
    return {.x = p.x * factor, .y = p.y * factor};
  }

  /// Euclidean norm (equation 2 when applied to a difference of positions).
  [[nodiscard]] inline double norm(point const & p) noexcept {
    return std::sqrt(p.x * p.x + p.y * p.y);
  }

  inline std::istream & operator>>(std::istream & is, point & p) {
    is >> p.x >> p.y;
    return is;
  }

}  // namespace common

template <> struct std::formatter<common::point> : public std::formatter<std::string_view> {
  auto format(common::point const & p, std::format_context & ctx) const {
    std::string const text = std::format("{} {}", p.x, p.y);
    return std::formatter<std::string_view>::format(text, ctx);
  }
};

#endif  // COMMON_POINT_HPP
