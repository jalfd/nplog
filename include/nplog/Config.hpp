#ifndef NP_CONFIG_HPP
#define NP_CONFIG_HPP

#include <string_view>

namespace np {
  struct Config {
    Config();

    setLevels(unsigned char messages, unsigned char parameters);
    setLevelForName(std::string_view name, unsigned char messages, unsigned char parameters);
    setLevelForDepth(unsigned int depth, unsigned char messages, unsigned char parameters);

    void apply() const;

  private:
    std::unique_ptr<LogConfig> cfg;
  };

  // I am logger N, I am nested at depth D, and my most recent configuration is from version V
  bool isCurrent(unsigned V);
  std::pair<Levels, unsigned> getLevels(std::string_view N, unsigned D);
} // namespace np
#endif
