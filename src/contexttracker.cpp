#include "contexttracker.hpp"
#include <nplog/serializer.hpp>
#include "messagebuffer.hpp"
#include "stringinterner.hpp"

namespace np::log {
  ContextTracker::ContextTracker() = default;
  ContextTracker::~ContextTracker() {
    for (const auto& [key, value] : context_entries) {
      releaseBuffer(value);
    }
  }

  ContextId ContextTracker::allocate(const char* key, ValueSerializer* vs) {
    auto buf = acquireBuffer();
    const auto id = context_entries.size();
    context_entries.push_back(std::pair{global_interner.intern(key), buf});
    *vs = ValueSerializer(buf);
    return id;
  }

  void ContextTracker::release(ContextId id) {
    if (id >= context_entries.size()) { return; }

    context_entries.resize(id);
    clearCachedContext();
  }

  bool ContextTracker::hasContext() const noexcept
  {
    return !context_entries.empty();
  }

  const MessageBuffer& ContextTracker::context() {
    const auto new_entries = context_entries.size() - dirty_index;
    // are there any unserialized entries
    if (new_entries > 0) {
      // first, gather a list of new keys
      std::vector<std::pair<std::string_view, bool>> keys;
      keys.resize(new_entries);
      std::transform(context_entries.begin()
          + static_cast<decltype(context_entries)::difference_type>(dirty_index),
        context_entries.end(),
        std::back_inserter(keys),
        [](auto pair) { return std::pair{pair.first, false}; });

      std::sort(keys.begin(), keys.end(), [](auto lhs, auto rhs) { return lhs.first < rhs.first; });
      keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

      const auto has_conflict = [&]() {
        // now, see if any of these conflict with any of the existing keys
        for (size_t i = 0, j = 0; i < keys.size() && j < serialized_keys.size();) {
          const auto new_key = keys[i].first;
          const auto existing_key = serialized_keys[i];
          if (new_key == existing_key) { return true; }

          if (new_key < existing_key) {
            ++i;
          } else {
            ++j;
          }
        }
        return false;
      }();

      if (has_conflict) {
        // clear serialized cache and retry
        clearCachedContext();
        return context();
      }

      // finally, append new entries
      Serializer s(&buffer, dirty_index != 0);
      std::for_each(context_entries.begin()
          + static_cast<decltype(context_entries)::difference_type>(dirty_index),
        context_entries.end(),
        [&s](const auto pair) {
          s.writeJsonKey(pair.first);
          s.valueSerializer().writeLiteral(pair.second->contents());
        });

      dirty_index = context_entries.size();
    }

    return buffer;
  }

  void ContextTracker::clearCachedContext() noexcept {
    dirty_index = 0;
    buffer.clear();
    serialized_keys.clear();
  }

  ContextTracker& contextTracker() {
    thread_local ContextTracker context_tracker;
    return context_tracker;
  }

  ContextId allocateContext(const char* key, ValueSerializer* vs) {
    return contextTracker().allocate(key, vs);
  }
  void releaseContext(ContextId id) { contextTracker().release(id); }
} // namespace np::log