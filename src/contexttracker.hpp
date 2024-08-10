#ifndef NP_CONTEXTTRACKER_HPP
#define NP_CONTEXTTRACKER_HPP

#include <vector>
#include <nplog/export.hpp>
#include <nplog/scopedcontext.hpp>
#include "messagebuffer.hpp"

namespace np::log {
  struct LogProp;

  class NPLOG_EXPORT ContextTracker {
  public:
    ContextTracker();
    ~ContextTracker();
    ContextId allocate(const char* key, ValueSerializer* vs);
    void release(ContextId id);

    const MessageBuffer& context();

  private:
    void clearCachedContext() noexcept;
    std::vector<std::pair<const char*, MessageBuffer*>> context_entries;
    std::vector<const char*> serialized_keys;
    MessageBuffer buffer;
    size_t dirty_index = 0;
  };
} // namespace np::log
#endif