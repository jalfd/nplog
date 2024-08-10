#include <nplog/loggroup.hpp>
#include "messagebuffer.hpp"
#include <nplog/serializer.hpp>
#include "configimpl.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>

namespace np::log {
  LogGroup::LogGroup(LogGroup* parent, const char* name) noexcept
    : parent(parent)
    , name_ptr(name)
    , name_len(name_ptr ? strlen(name_ptr) : 0)
    , depth(parent ? parent->depth + 1 : 0) {
    refreshLevels(0, currentVersion());
  }

  LogGroup::LogGroup(const char* name) noexcept : LogGroup(nullptr, name) {}

  LogGroup::~LogGroup() noexcept = default;

  LevelSpec LogGroup::refreshLevels(unsigned version_, unsigned global_version, bool exclude_depth) noexcept {
    if (!isCurrent(version_, global_version)) {
      auto result = getLevels(std::string_view(name_ptr, name_len), depth);
      levels_by_name_only = result.levels_by_name_only;
      effective_levels = result.effective_levels;

      if (parent) {
        auto parent_levels = parent->refreshLevels(result.version, global_version, true);
        effective_levels = merge(effective_levels, parent_levels);
        levels_by_name_only = merge(levels_by_name_only, parent_levels);
      }
      version = result.version;
    }
    return exclude_depth ? levels_by_name_only : effective_levels;
  }
} // namespace np::log
