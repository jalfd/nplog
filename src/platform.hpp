#ifndef NP_PLATFORM_HPP
#define NP_PLATFORM_HPP

#include <cstdint>
#include <string>

namespace np::platform {
  uint64_t processId();
  uint64_t threadId();
  std::string hostname();
  std::string executableName();
} // namespace np::platform

#endif
