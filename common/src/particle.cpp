#include "particle.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <memory>
#include <system_error>

namespace common {

  namespace {

    constexpr std::string_view blanks         = " \t\r\v\f";
    constexpr std::size_t fields_per_particle = 6;

    /// Removes and returns the next blank-separated token of line (empty when none is left).
    std::string_view next_token(std::string_view & line) {
      auto const begin = line.find_first_not_of(blanks);
      if (begin == std::string_view::npos) {
        line = {};
        return {};
      }
      line.remove_prefix(begin);
      auto const length = std::min(line.find_first_of(blanks), line.size());
      auto const token  = line.substr(0, length);
      line.remove_prefix(length);
      return token;
    }

    /// Converts a whole token into a finite double.
    std::optional<double> parse_double(std::string_view token) {
      double value             = 0.0;
      auto const * const first = std::to_address(token.begin());
      auto const * const last  = std::to_address(token.end());
      auto const [ptr, ec]     = std::from_chars(first, last, value);
      if (ec != std::errc{} or ptr != last or not std::isfinite(value)) { return std::nullopt; }
      return value;
    }

  }  // namespace

  std::optional<particle> parse_particle(std::string_view line) {
    std::array<double, fields_per_particle> values{};
    for (auto & value : values) {
      auto const parsed = parse_double(next_token(line));
      if (not parsed) { return std::nullopt; }
      value = *parsed;
    }
    if (not next_token(line).empty()) { return std::nullopt; }
    auto const [mass, radius, x, y, vx, vy] = values;
    return particle{
      .id       = 0,
      .mass     = mass,
      .radius   = radius,
      .position = { .x = x,  .y = y},
      .velocity = {.x = vx, .y = vy},
    };
  }

  bool is_valid(particle const & p, enclosure const & box) noexcept {
    return p.mass > 0.0 and
           p.radius > 0.0 and
           p.position.x >= p.radius and
           p.position.x <= box.width - p.radius and
           p.position.y >= p.radius and
           p.position.y <= box.height - p.radius;
  }

}  // namespace common
