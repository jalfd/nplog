#ifndef NP_COMMON_HPP
#define NP_COMMON_HPP

#include <cstdint>
#include <nplog/export.hpp>

namespace np::log {
  using level_type = uint16_t;

  // TODO: Bad name
  struct NPLOG_EXPORT LevelWrapper {
    explicit LevelWrapper(level_type l) noexcept;
    level_type l;
  };

  NPLOG_EXPORT LevelWrapper mask(level_type) noexcept;
  NPLOG_EXPORT LevelWrapper threshold(level_type) noexcept;// make this the default? Or explicit?

  struct NPLOG_EXPORT LevelSpec {
    LevelSpec() noexcept = default;
    LevelSpec(LevelWrapper message, LevelWrapper props) noexcept;
    LevelSpec(LevelWrapper message) noexcept;
    level_type message = {};
    level_type props = {};
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

  NPLOG_EXPORT unsigned currentVersion() noexcept;

  struct ScopedMessage;
  bool NPLOG_EXPORT suppressProp(const ::np::log::ScopedMessage& sm, level_type, level_type prop_level) noexcept;
} // namespace np::log

#endif
