#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <nplog/export.hpp>
#include <nplog/formatter.hpp>
#include <nplog/serializer.hpp>
#include <memory>
#include <stddef.h>

namespace np::log {
  using ContextId = size_t;

  NPLOG_EXPORT ContextId allocateContext(const char* key, ValueSerializer* vs);
  NPLOG_EXPORT void releaseContext(ContextId id);

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
} // namespace np::log
#endif