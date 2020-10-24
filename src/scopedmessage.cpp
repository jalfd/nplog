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
    std::string_view logger_params_data)
    : message_buffer(buffer), serializer(buffer), message_level(level) {
    serializer.prologue(file, line, enabled_fields, level, log_name, m);
    if (!logger_params_data.empty()) {
      serializer.startObject("static");
      serializer.valueSerializer().writeLiteral(logger_params_data);
      serializer.endObject();
    }
  }

  void ScopedMessageBase::endMessage() {
    if (has_params) { serializer.endObject(); }
    serializer.epilogue();
  }
  ScopedMessage::ScopedMessage(Logger& log,
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
      log.name(),
      log.loggerParams() ? log.loggerParams()->data.contents() : std::string_view())
    , log(log), global_version(global_version) {}

  ScopedMessage::~ScopedMessage() {
    endMessage();
    log.submitMessage(message_level, *message_buffer, global_version);
    releaseBuffer(message_buffer);
  }
} // namespace np::log
