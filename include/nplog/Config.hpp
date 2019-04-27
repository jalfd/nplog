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

    void setLevelForLogName(std::string_view logname, level_type messages, level_type params);
    void setLevelForLogDepth(unsigned depth, level_type messages, level_type params);

    void apply() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };

  NPLOG_EXPORT void setSink(std::function<void(level_type, std::string_view msg)> sink);
} // namespace np
#endif
