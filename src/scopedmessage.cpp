#include <nplog/scopedmessage.hpp>
#include "configimpl.hpp"

namespace np::log {
  ScopedMessageBase::ScopedMessageBase(std::string_view file,
    int line,
    Config::Fields enabled_fields,
    level_type level,
    std::string_view m,
    MessageBuffer* buffer,
    std::string_view log_name,
    std::string_view group_props_data)
    : message_buffer(buffer), serializer(buffer), message_level(level) {
    serializer.prologue(file, line, enabled_fields, level, log_name, m);
    if (!group_props_data.empty()) {
      serializer.startObject("group");
      serializer.valueSerializer().writeLiteral(group_props_data);
      serializer.endObject();
    }
  }

  void ScopedMessageBase::endMessage() {
    if (has_params) { serializer.endObject(); }
    serializer.epilogue();
  }
  ScopedMessage::ScopedMessage(LogGroup& group,
    std::string_view file,
    int line,
    unsigned global_version,
    level_type level,
    std::string_view m)
    : ScopedMessageBase(file,
      line,
      enabledFields(global_version),
      level,
      m,
      acquireBuffer(),
      group.name(),
      group.props() ? group.props()->data.contents() : std::string_view())
    , global_version(global_version) {}

  ScopedMessage::~ScopedMessage() {
    endMessage();
    ::np::log::sendToSink(message_level, message_buffer->contents(), global_version);
    releaseBuffer(message_buffer);
  }
} // namespace np::log
