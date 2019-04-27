#ifndef NP_COMMON_HPP
#define NP_COMMON_HPP

#include <cstdint>

namespace np {
  using level_type = uint16_t;

  inline bool testLevel(level_type level, level_type mask) {
    const auto userlevel = level & 0xff00;
    const auto standardlevel = level & 0x00ff;
    const auto usermask = mask & 0xff00;
    const auto standardmask = mask & 0x00ff;

    return (userlevel & usermask) == userlevel && standardlevel <= standardmask;
  }
} // namespace np

#endif
