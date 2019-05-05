#ifndef NP_CONFIG_HPP
#define NP_CONFIG_HPP

#include <nplog/common.hpp>
#include <nplog/export.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string_view>

namespace np {
  struct Levels {
    level_type message = {};
    level_type param = {};
  };

  struct NPLOG_EXPORT Config {
    Config(level_type message_level, level_type param_level);
    ~Config();
    Config(const Config&) = delete;
    Config(Config&&) = delete;

    void setLevelForLogName(std::string_view logname, level_type messages, level_type params);
    void setLevelForLogDepth(unsigned depth, level_type messages, level_type params);

    void apply() const;

  private:
    struct Impl;
    Impl* impl;
  };

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

  NPLOG_EXPORT void setHeaderFields(Fields fields_mask);

  NPLOG_EXPORT void setSink(std::function<void(level_type, std::string_view msg)> sink);
} // namespace np
#endif
