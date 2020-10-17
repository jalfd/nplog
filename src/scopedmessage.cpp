#include <nplog/scopedmessage.hpp>
#include "configimpl.hpp"

namespace np::log {
  ScopedMessageBase::ScopedMessageBase(const char* file,
    int line,
    level_type level,
    const char* m,
    MessageBuffer buffer,
    std::string_view log_name,
    std::string_view logger_params_data)
    : message_buffer(std::move(buffer)), serializer(&message_buffer), message_level(level) {
    serializer.prologue(file, line, level, log_name, m);
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
    const char* file,
    int line,
    level_type level,
    const char* m)
    : ScopedMessageBase(file,
      line,
      level,
      m,
      acquireBuffer(),
      log.name(),
      log.loggerParams() ? log.loggerParams()->data.contents() : std::string_view())
    , log(log) {}

  ScopedMessage::~ScopedMessage() {
    endMessage();
    log.submitMessage(message_level, message_buffer);
    releaseBuffer(std::move(message_buffer));
  }
} // namespace np::log
