#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <nplog/export.hpp>
#include <nplog/formatter.hpp>
#include <nplog/serializer.hpp>
#include <memory>
#include <stddef.h>
namespace np::log {
  struct MessageBuffer;
  struct ContextSnapshotData;
  using ContextSnapshot = std::shared_ptr<ContextSnapshotData>;
  using ContextId = uintptr_t;

  NPLOG_EXPORT ContextId allocateContext(const char* key, ValueSerializer* vs);
  NPLOG_EXPORT void releaseContext(ContextId id);
  NPLOG_EXPORT ContextId attachSnapshot(ContextSnapshot);
  NPLOG_EXPORT void detachSnapshot(ContextId);

  struct ScopedContext {
    template <typename T>
    ScopedContext(const char* key, T&& value) {
      ValueSerializer vs;
      id = allocateContext(key, &vs);
      np::log::format(value, vs);
    }

    ~ScopedContext() { releaseContext(id); }

  private:
    ContextId id;
  };

  struct ScopedContextSnapshot {
    ScopedContextSnapshot(ContextSnapshot s) : snapshot(s) {
      id = attachSnapshot(s);
    }

    ~ScopedContextSnapshot() { detachSnapshot(id); }

    static ContextSnapshot current();

  private:
    ContextSnapshot snapshot;
    ContextId id;
  };
} // namespace np::log
#endif