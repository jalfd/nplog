#include "contexttracker.hpp"
#include <nplog/serializer.hpp>
#include "messagebuffer.hpp"

namespace np::log {
    ContextTracker::ContextTracker() = default;
    ContextTracker::~ContextTracker() {
        for (const auto &[key, value] : context_entries) {
            releaseBuffer(value);
        }
    }


  ContextId ContextTracker::allocate(const char* key, ValueSerializer* vs) {
    dirty = true;
    auto buf = acquireBuffer();
    const auto id = reinterpret_cast<ContextId>(key);
    context_entries.insert(std::pair{id, buf});
    context_entries[id] = buf;
    *vs = ValueSerializer(buf);
    return id;
  }

  ContextId ContextTracker::attachSnapshot(MessageBuffer* snapshot) {
    const auto id = reinterpret_cast<uintptr_t>(snapshot);
    context_entries.insert(std::pair{id, snapshot});
    return id;
  }

  void ContextTracker::release(ContextId id) {
    const auto it = context_entries.find(id);
    if (it == context_entries.end()) { return;}
    dirty = true;
    releaseBuffer(it->second);
    context_entries.erase(it);
  }

  const MessageBuffer& ContextTracker::context() {
    if (dirty) {
      buffer.clear();
      Serializer s(&buffer);
      for (const auto& [key, value] : context_entries) {
        if (key == reinterpret_cast<uintptr_t>(value)) {
          // we have a snapshot, just copy the whole thing
          s.valueSerializer().writeLiteral(value->contents());
        } else {
          s.writeKey(reinterpret_cast<const char*>(key));
          s.valueSerializer().writeLiteral(value->contents());
        }
      }
      dirty = false;
    }

    return buffer;
  }

  thread_local ContextTracker context_tracker;

  ContextId allocateContext(const char* key, ValueSerializer* vs) {
    return context_tracker.allocate(key, vs);
  }
  void releaseContext(ContextId id) { context_tracker.release(id); }

  ContextId attachSnapshot(ContextSnapshot snapshot) {
    return context_tracker.attachSnapshot(snapshot.get());
  }
  void detachSnapshot(ContextId id) { context_tracker.release(id); }
} // namespace np::log