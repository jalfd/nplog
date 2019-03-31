#ifndef NP_CONFIG_HPP
#define NP_CONFIG_HPP

#include <nplog/export.hpp>
#include <string_view>
#include <memory>
#include <map>

namespace np {
  struct Levels {
    int message = 0;
    int param = 0;
  };

  struct NPLOG_EXPORT Config {
    Config(int message_level, int param_level);
    ~Config();

    void setLevelForLogName(std::string_view logname, int messages, int params);
    void setLevelForLogDepth(unsigned depth, int messages, int params);

    void apply() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
  };

  NPLOG_EXPORT void setSink(std::function<void(int, std::string_view msg)> sink);

  inline bool suppressMessage(Levels lvl, int message) {
      return message > lvl.message;
  }
} // namespace np
#endif
