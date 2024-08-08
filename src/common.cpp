#include <nplog/common.hpp>

namespace np::log {
  LevelSpec::LevelSpec(level_type m, level_type p) noexcept
    : message(m), props(p) {}
  LevelSpec::LevelSpec(level_type message) noexcept : LevelSpec(message, message) {}
} // namespace np::log
