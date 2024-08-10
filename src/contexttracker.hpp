#ifndef NP_CONTEXTTRACKER_HPP
#define NP_CONTEXTTRACKER_HPP

#include <vector>
#include <nplog/export.hpp>
#include <unordered_map>
#include <nplog/scopedcontext.hpp>
#include "messagebuffer.hpp"

namespace np::log {
  struct LogProp;
  struct ContextSnapshotData {
    std::vector<std::pair<const ContextId, MessageBuffer*>> entries;
  };

  class NPLOG_EXPORT ContextTracker {
  public:
    ContextTracker();
    ~ContextTracker();
    ContextId allocate(const char* key, ValueSerializer* vs);
    ContextId attachSnapshot(MessageBuffer* snapshot);
    void release(ContextId id);

    const MessageBuffer& context();
    std::shared_ptr<ContextSnapshotData> snapshot();

  private:
    std::unordered_map<ContextId, MessageBuffer*> context_entries;
    MessageBuffer buffer;
    bool dirty = false;
  };
} // namespace np::log
#endif