#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <nplog/formatter.hpp>
#include <nplog/loggroup.hpp>
#include <string_view>

namespace np::log {
  struct NPLOG_EXPORT ScopedMessageBase {
    ScopedMessageBase(std::string_view file,
      int line,
      Fields enabled_fields,
      level_type level,
      std::string_view m,
      MessageBuffer* buffer,
      std::string_view log_name,
      std::string_view group_props_data);

    void endMessage();

    template <typename T>
    bool addParam(std::string_view name, T&& expr) {
      if (!has_params) {
        has_params = true;
        serializer.startObject("params");
      }
      serializer.writeKey(name);
      auto vs = serializer.valueSerializer();
      np::log::format(expr, vs);
      return true;
    }

    const MessageBuffer& buffer() { return *message_buffer; }

  protected:
    MessageBuffer* message_buffer;
    Serializer serializer;

    level_type message_level;
    bool has_params = false;
  };

  struct NPLOG_EXPORT ScopedMessage : private ScopedMessageBase {
    ScopedMessage(LogGroup& group, std::string_view file, int line, unsigned global_version, level_type level, std::string_view m);

    ~ScopedMessage();

    using ScopedMessageBase::addParam;

  private:
    unsigned global_version;
  };
} // namespace np::log
#endif
