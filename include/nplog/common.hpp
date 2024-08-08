#ifndef NP_COMMON_HPP
#define NP_COMMON_HPP

#include <cstdint>
#include <nplog/export.hpp>

namespace np::log {
  using level_type = unsigned char;

  struct NPLOG_EXPORT LevelSpec {
    LevelSpec() noexcept = default;
    LevelSpec(level_type message, level_type props) noexcept;
    LevelSpec(level_type message) noexcept;
    level_type message = {};
    level_type props = {};
  };

  enum Levels : level_type {
    Fatal = 0,
    Error = 16,
    Warning = 32,
    Info = 48,
    Debug = 64,
    Trace = 80
  };

  // FIXME: enum class, now that it's a free ? What?
  // but also, why 32 bits?
    enum Fields : uint16_t {
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

  NPLOG_EXPORT unsigned currentVersion() noexcept;

  inline bool testLevel(level_type level, level_type threshold) noexcept { return level <= threshold; }

  inline bool suppressProp(LevelSpec threshold, level_type, level_type prop_level) noexcept {
    return !testLevel(prop_level, static_cast<level_type>(threshold.props));
  }
  inline bool suppressProp(LevelSpec threshold, level_type msg_level, const char* = nullptr) noexcept {
    return suppressProp(threshold, msg_level, msg_level);
  }

} // namespace np::log

#endif
