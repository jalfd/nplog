#ifndef NP_PLATFORM_HPP
#define NP_PLATFORM_HPP

#include <cstdint>
#include <string>

namespace np::log::platform {
  uint64_t processId() noexcept;
  uint64_t threadId() noexcept;
  std::string hostname() noexcept;
  std::string executableName() noexcept;
} // namespace np::log::platform

#endif
