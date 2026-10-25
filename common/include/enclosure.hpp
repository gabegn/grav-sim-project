#ifndef COMMON_ENCLOSURE_HPP
#define COMMON_ENCLOSURE_HPP

namespace common {

  /// Rectangular simulation space spanning [0, width] x [0, height].
  struct enclosure {
    double width  = 0.0;
    double height = 0.0;
  };

}  // namespace common

#endif  // COMMON_ENCLOSURE_HPP
