#ifndef NP_CONTEXTTRACKER_HPP
#define NP_CONTEXTTRACKER_HPP

#include <nplog/export.hpp>
#include <unordered_map>
#include <nplog/scopedcontext.hpp>
#include "messagebuffer.hpp"

namespace np::log {
  struct LogProp;

  class NPLOG_EXPORT ContextTracker {
  public:
    ContextTracker();
    ~ContextTracker();
    ContextId allocate(const char* key, ValueSerializer* vs);
    ContextId attachSnapshot(MessageBuffer* snapshot);
    void release(ContextId id);

    const MessageBuffer& context();

  private:
    std::unordered_map<ContextId, MessageBuffer*> context_entries;
    MessageBuffer buffer;
    bool dirty = false;
  };
} // namespace np::log
#endif