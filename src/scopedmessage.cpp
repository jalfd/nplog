#include <nplog/scopedmessage.hpp>
#include "messagebuffer.hpp"
#include "configimpl.hpp"
#include "loggroupprops.hpp"
#include "stringinterner.hpp"
#include "contexttracker.hpp"

namespace np::log {
  ScopedMessageBase::ScopedMessageBase(std::string_view file,
    int line,
    Fields enabled_fields,
    level_type level,
    const char* m,
    MessageBuffer* buffer,
    std::string_view log_name,
    std::string_view group_props_data) noexcept
    : message_buffer(buffer), serializer(buffer), message_level(level) {
    serializer.prologue(file, line, enabled_fields, level, log_name, global_interner.intern(m));
    if (!group_props_data.empty()) {
      serializer.startObject("group");
      serializer.valueSerializer().writeLiteral(group_props_data);
      serializer.endObject();
    }
    const auto& context = contextTracker().context();
    if (!context.contents().empty()) {
      serializer.startObject("context");
      serializer.valueSerializer().writeLiteral(context.contents());
      serializer.endObject();
    }
  }

  void ScopedMessageBase::endMessage() noexcept {
    if (has_props) { serializer.endObject(); }
    serializer.epilogue();
  }

  ValueSerializer ScopedMessageBase::startProp(std::string_view name) noexcept {
    if (!has_props) {
      has_props = true;
      serializer.startObject("props");
    }
    serializer.writeJsonKey(global_interner.intern(name.data()));
    return serializer.valueSerializer();
  }
  ScopedMessage::ScopedMessage(LogGroup& group,
    std::string_view file,
    int line,
    unsigned global_version,
    level_type level,
    const char* m) noexcept
    : ScopedMessageBase(file,
      line,
      enabledFields(global_version),
      level,
      m,
      acquireBuffer(),
      group.name(),
      group.props() ? group.props()->data.contents() : std::string_view())
    , global_version(global_version) {}

  ScopedMessage::~ScopedMessage() noexcept {
    endMessage();
    ::np::log::sendToSink(message_level, message_buffer->contents(), global_version);
    releaseBuffer(message_buffer);
  }
} // namespace np::log
