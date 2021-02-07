#include <nplog/loggroup.hpp>
#include "messagebuffer.hpp"
#include <nplog/serializer.hpp>
#include "configimpl.hpp"
#include "loggroupprops.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>

namespace np::log {
  uint32_t LogProp::serialize(Serializer& serializer,
    MessageBuffer& buffer,
    const char* name,
    const void* value,
    void (*format_func)(const void*, ValueSerializer&)) noexcept {
    serializer.writeKey(name);
    const auto name_end = buffer.messageSize(); // FIXME: do we really want to depend on buffer? No.
                                                // We don't. Absolutely not.
    // It's still a bit iffy and magical that we just return a weird middle-of-the-string pointer
    auto vs = serializer.valueSerializer();
    format_func(value, vs);
    return static_cast<uint32_t>(name_end);
  }

  LogGroupProps::LogGroupProps(LogGroupProps* parent, std::initializer_list<LogProp> props) noexcept {
    Serializer s(&data);
    std::vector<const LogProp*> prop_ptrs; // FIXME: should reserve
    std::transform(
      props.begin(), props.end(), std::back_inserter(prop_ptrs), [](const LogProp& p) {
        return &p;
      });
    std::sort(prop_ptrs.begin(), prop_ptrs.end(), [](const LogProp* lhs, const LogProp* rhs) {
      return strcmp(lhs->name, rhs->name) < 0;
    });
    for (const auto& prop_ptr : prop_ptrs) {
      const auto start = static_cast<uint32_t>(data.messageSize());
      const auto name_end = prop_ptr->serialize(s, data, prop_ptr->name, prop_ptr->value, prop_ptr->format_func);
      const auto end = static_cast<uint32_t>(data.messageSize());
      offsets.emplace_back(start == 0 ? start : start + 1, name_end, end);
    }

    // Merge with parent's props
    if (parent) {
      auto parent_it = parent->offsets.begin();
      auto self_it = offsets.begin();

      // ok, so we need a new buffer
      LogGroupProps dest; // should reserve member size

      const auto copy_to_dest
        = [&dest](const LogGroupProps& src, size_t first, size_t name_last, size_t last) {
            const auto new_first = dest.data.messageSize();
            const auto span
              = src.data.contents().substr(first, last - first); // the span of characters to copy
            auto* insert_at = dest.data.insertAt(last - first);
            span.copy(insert_at, span.size(), 0);
            dest.data.append(',');

            dest.offsets.emplace_back(static_cast<uint32_t>(new_first),
              static_cast<uint32_t>(new_first + (name_last - first)),
              static_cast<uint32_t>(new_first + (last - first)));
          };

      while (parent_it != parent->offsets.end() && self_it != offsets.end()) {
        const auto parent_name = std::string_view(parent->data.contents().substr(
          std::get<0>(*parent_it), std::get<1>(*parent_it) - std::get<0>(*parent_it)));
        const auto self_name = std::string_view(data.contents().substr(
          std::get<0>(*self_it), std::get<1>(*self_it) - std::get<0>(*self_it)));

        bool pick_from_parent = parent_name < self_name;
        auto* current = pick_from_parent ? parent : this;
        auto& current_it = pick_from_parent ? parent_it : self_it;
        copy_to_dest(
          *current, std::get<0>(*current_it), std::get<1>(*current_it), std::get<2>(*current_it));
        if (parent_name == self_name) {
          ++current_it;
          ++parent_it;
        } else {
          ++current_it;
        }
      }

      // whichever range has stuff left, just copy it in
      for (; parent_it != parent->offsets.end(); ++parent_it) {
        copy_to_dest(
          *parent, std::get<0>(*parent_it), std::get<1>(*parent_it), std::get<2>(*parent_it));
      }
      for (; self_it != offsets.end(); ++self_it) {
        copy_to_dest(*this, std::get<0>(*self_it), std::get<1>(*self_it), std::get<2>(*self_it));
      }
      dest.data.shrinkTo(dest.data.messageSize() - 1);
      *this = std::move(dest);
    }
  }

  LogGroup::LogGroup(LogGroup* parent, const char* name) noexcept
    : parent(parent)
    , group_props(parent ? parent->group_props : nullptr)
    , name_ptr(name)
    , name_len(name_ptr ? strlen(name_ptr) : 0)
    , depth(parent ? parent->depth + 1 : 0) {
    refreshLevels(0, currentVersion());
  }

  LogGroup::LogGroup(const char* name) noexcept : LogGroup(nullptr, name) {}

  LogGroup::LogGroup(LogGroup* parent, const char* name, std::initializer_list<LogProp> props) noexcept
    : LogGroup(parent, name) {
    if (props.size() != 0) {
      group_props = new LogGroupProps(parent ? parent->group_props : nullptr, props);
    }
  }

  LogGroup::~LogGroup() noexcept {
    // Delete, unless we're pointing at our parent's props
    if (!(parent && parent->group_props == group_props)) { delete group_props; }
  }

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
