#ifndef NP_COMMON_HPP
#define NP_COMMON_HPP

#include <cstdint>
#include <nplog/export.hpp>

namespace np::log {
  using level_type = uint16_t;

  struct LevelSpec {
    level_type message = {};
    level_type param = {};
  };

  enum Levels : uint16_t {
    Fatal = 1,
    Error = 2,
    Warning = 4,
    Status = 8,
    DebugHigh = 16,
    DebugMid = 32,
    DebugLow = 64,
    Trace = 128
  };

  // FIXME: enum class, now that it's a free
    enum Fields : uint32_t {
      File = 1,
      Line = 2,
      Time = 4,
      Level = 8,
      LevelName = 16,
      LogName = 32,
      ProcessName = 64,
      ProcessId = 128,
      ThreadId = 256,
      Hostname = 512,
    };

  NPLOG_EXPORT unsigned currentVersion();

  inline bool testLevel(level_type level, level_type mask) { return (level & mask) == level; }

  inline bool suppressParam(LevelSpec level, level_type, level_type param_level) {
    return !testLevel(param_level, static_cast<level_type>(level.param));
  }
  inline bool suppressParam(LevelSpec level, level_type msg_level, const char* = nullptr) {
    return suppressParam(level, msg_level, msg_level);
  }

} // namespace np::log

#endif
