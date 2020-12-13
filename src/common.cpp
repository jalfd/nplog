#include <nplog/common.hpp>

namespace np::log {
  LevelWrapper::LevelWrapper(level_type l) : l(l) {}

  LevelWrapper mask(level_type l) { return LevelWrapper(l); }
  LevelWrapper threshold(level_type l) { return LevelWrapper(static_cast<level_type>(l * 2 - 1)); }

  LevelSpec::LevelSpec(LevelWrapper message, LevelWrapper param)
    : message(message.l), param(param.l) {}
  LevelSpec::LevelSpec(LevelWrapper message) : LevelSpec(message, message) {}
} // namespace np::log
