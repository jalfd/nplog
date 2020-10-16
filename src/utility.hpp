#ifndef NP_UTILITY_HPP
#define NP_UTILITY_HPP

#include <algorithm>
#include <cstdlib>
#include <string_view>

namespace np::log {
  template <typename T>
  size_t to_size_t_checked(T val) {
#ifdef NP_CHECK_CONVERSIONS
    if (val < 0) { abort(); }
#endif
    return static_cast<size_t>(val);
  }

  inline std::string_view filenameFromPath(std::string_view path) {
    const auto it
      = std::find_if(path.rbegin(), path.rend(), [](char c) { return c == '/' || c == '\\'; });
    if (it != path.rend()) { path.remove_prefix(to_size_t_checked(path.rend() - it)); }
    return path;
  }

  inline std::string_view hostnameFromFqdn(std::string_view path) {
    const auto it = std::find(path.begin(), path.end(), '.');
    if (it != path.end()) { path.remove_suffix(to_size_t_checked(path.end() - it)); }
    return path;
  }
} // namespace np::log

#endif
