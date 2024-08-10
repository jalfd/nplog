#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <nplog/formatter.hpp>
#include <nplog/loggroup.hpp>
#include <nplog/serializer.hpp>
#include <string_view>

namespace np::log {
  struct NPLOG_EXPORT ScopedMessageBase {
    ScopedMessageBase(std::string_view file,
      int line,
      Fields enabled_fields,
      level_type level,
      const char* m,
      MessageBuffer* buffer,
      std::string_view log_name,
      std::string_view group_props_data) noexcept;

    void endMessage() noexcept;

    template <typename T>
    bool addProp(std::string_view name, T&& expr) noexcept {
      auto vs = startProp(name);
      np::log::format(expr, vs);
      return {};
    }

    ValueSerializer startProp(std::string_view name) noexcept;

  protected:
    MessageBuffer* message_buffer;
    Serializer serializer;

    level_type message_level;
    bool has_props = false;
  };

  struct NPLOG_EXPORT ScopedMessage : private ScopedMessageBase {
    ScopedMessage(LogGroup& group, std::string_view file, int line, unsigned global_version, level_type level, const char* m) noexcept;

    ~ScopedMessage() noexcept;

    using ScopedMessageBase::addProp;

  private:
    unsigned global_version;
  };
} // namespace np::log
#endif
