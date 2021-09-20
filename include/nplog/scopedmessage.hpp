#ifndef NP_LOG_SCOPEDMESSAGE_HPP
#define NP_LOG_SCOPEDMESSAGE_HPP

#include <nplog/formatter.hpp>
#include <nplog/loggroup.hpp>
#include <nplog/serializer.hpp>
#include <string_view>
#include <memory>

namespace np::log {
    struct LogConfig;

    // TODO: move to common?
    struct source_location {
        constexpr std::uint_least32_t line() const noexcept { return _line; }
        constexpr const char* file_name() const noexcept { return _file.data(); }

        const std::uint_least32_t _line;
        const std::string_view _file;
    };

  struct NPLOG_EXPORT ScopedMessageBase {
    void beginMessage(MessageBuffer* buffer,
      source_location loc,
      level_type level,
      Fields enabled_fields,
      std::string_view message,
      std::string_view group_name,
      std::string_view group_props) noexcept;

    void endMessage() noexcept;

    template <typename T>
    bool addProp(std::string_view name, T&& expr) noexcept {
      auto vs = startProp(name);
      np::log::format(expr, vs);
      return {};
    }

  private:
    ValueSerializer startProp(std::string_view name) noexcept;
    bool has_props = false;

  protected:
    Serializer serializer = Serializer(nullptr);
  };

  struct NPLOG_EXPORT ScopedMessage : private ScopedMessageBase {
    ScopedMessage(LogGroup& group, level_type level) noexcept;

    ~ScopedMessage() noexcept;

    // check if the message should be logged
    explicit operator bool() const noexcept;

    void write(source_location loc, std::string_view message) noexcept;

    using ScopedMessageBase::addProp;

  private:
    std::shared_ptr<LogConfig> config_ptr;
    LogGroup* log_group;
    unsigned global_version;
    level_type message_level;
    LevelSpec thresholds;

    friend bool suppressProp(const ScopedMessage&, level_type, level_type) noexcept;
  };
} // namespace np::log
#endif
