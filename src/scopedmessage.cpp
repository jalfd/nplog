#include <nplog/scopedmessage.hpp>
#include "configimpl.hpp"
#include "loggroupprops.hpp"
#include "messagebuffer.hpp"

namespace np::log {
    namespace {
        inline bool testLevel(level_type level, level_type mask) noexcept { return (level & mask) == level; }
    }
  void ScopedMessageBase::beginMessage(MessageBuffer* buffer,
    source_location loc,
    level_type level,
    Fields enabled_fields,
    std::string_view m,
    std::string_view log_name,
    std::string_view group_props_data) noexcept {
    serializer = Serializer(buffer);
    serializer.prologue(loc.file_name(), static_cast<int>(loc.line()), enabled_fields, level, log_name, m); // TODO: ditch static_cast for line conversion
    if (!group_props_data.empty()) {
      serializer.startObject("group");
      serializer.valueSerializer().writeLiteral(group_props_data);
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
    serializer.writeKey(name);
    return serializer.valueSerializer();
  }

  ScopedMessage::ScopedMessage(LogGroup& group, level_type level) noexcept
    : log_group(&group)
    , global_version(np::log::currentVersion())
    , message_level(level)
    , thresholds(log_group->refreshLevels(log_group->knownVersion(), global_version)) {}

  ScopedMessage::~ScopedMessage() noexcept {
    if (config_ptr) {
      endMessage();
      ::np::log::sendToSink(message_level, serializer.getBuffer()->contents(), global_version);
      releaseBuffer(serializer.getBuffer());
    }
  }

  ScopedMessage::operator bool() const noexcept {
    return testLevel(message_level, thresholds.message);
  }

  void ScopedMessage::write(source_location loc, std::string_view message) noexcept {
    config_ptr = getConfig(global_version);
    beginMessage(acquireBuffer(),
      loc,
      message_level,
      config_ptr->fields,
      message,
      log_group->name(),
      log_group->props() ? log_group->props()->data.contents() : std::string_view());
  }

  bool suppressProp(const ::np::log::ScopedMessage& sm, level_type, level_type prop_level) noexcept {
    return !testLevel(prop_level, static_cast<level_type>(sm.thresholds.props));
  }
} // namespace np::log
