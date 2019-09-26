#ifndef NP_UTILITY_HPP
#define NP_UTILITY_HPP

#include <algorithm>
#include <string_view>

namespace np::log {
  inline std::string_view filenameFromPath(std::string_view path) {
    const auto it
      = std::find_if(path.rbegin(), path.rend(), [](char c) { return c == '/' || c == '\\'; });
    if (it != path.rend()) { path.remove_prefix(path.rend() - it); }
    return path;
  }

  inline std::string_view hostnameFromFqdn(std::string_view path) {
    const auto it = std::find(path.begin(), path.end(), '.');
    if (it != path.end()) { path.remove_suffix(path.end() - it); }
    return path;
  }
} // namespace np

#endif
