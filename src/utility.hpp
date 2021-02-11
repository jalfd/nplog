#ifndef NP_UTILITY_HPP
#define NP_UTILITY_HPP

#include <algorithm>
#include <cstdlib>
#include <string_view>
#include <nplog/common.hpp>

namespace np::log {
  template <typename T>
  size_t to_size_t_checked(T val) noexcept {
#ifdef NP_CHECK_CONVERSIONS
    if (val < 0) { abort(); }
#endif
    return static_cast<size_t>(val);
  }

  inline std::string_view filenameFromPath(std::string_view path) noexcept {
    const auto it
      = std::find_if(path.rbegin(), path.rend(), [](char c) { return c == '/' || c == '\\'; });
    if (it != path.rend()) { path.remove_prefix(to_size_t_checked(path.rend() - it)); }
    return path;
  }

  inline std::string_view hostnameFromFqdn(std::string_view path) noexcept {
    const auto it = std::find(path.begin(), path.end(), '.');
    if (it != path.end()) { path.remove_suffix(to_size_t_checked(path.end() - it)); }
    return path;
  }

  namespace {
    inline bool testLevel(level_type level, level_type mask) noexcept {
      return (level & mask) == level;
    }
  } // namespace
} // namespace np::log

#endif
