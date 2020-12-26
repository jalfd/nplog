#include <nplog/common.hpp>

namespace np::log {
  LevelWrapper::LevelWrapper(level_type l) noexcept : l(l) {}

  LevelWrapper mask(level_type l) noexcept { return LevelWrapper(l); }
  LevelWrapper threshold(level_type l) noexcept { return LevelWrapper(static_cast<level_type>(l * 2 - 1)); }

  LevelSpec::LevelSpec(LevelWrapper message, LevelWrapper param) noexcept
    : message(message.l), param(param.l) {}
  LevelSpec::LevelSpec(LevelWrapper message) noexcept : LevelSpec(message, message) {}
} // namespace np::log
