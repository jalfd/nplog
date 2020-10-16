#ifndef NP_LOG_LOGGER_HPP
#define NP_LOG_LOGGER_HPP

#include <nplog/config.hpp>
#include <nplog/export.hpp>
#include <nplog/messagebuffer.hpp>
#include <nplog/serializer.hpp>

#include <string_view>

namespace np::log {
  struct NPLOG_EXPORT Logger {
    using buffer_type = MessageBuffer;
    using serializer_type = Serializer;

    explicit Logger(Logger* parent = nullptr, const char* name = nullptr);
    explicit Logger(const char* name);
    Logger(const Logger&) = delete;

    buffer_type acquireBuffer();

    void submitMessage(level_type level, buffer_type& buffer);

    void releaseBuffer(buffer_type&& buf);

    LevelSpec refreshLevels(unsigned version, bool exclude_depth = false);

    unsigned knownVersion() const { return version; }

    std::string_view name() const { return std::string_view(name_ptr, name_len); }

    bool permitSensitive() const { return sensitive; }

  private:
    LevelSpec effective_levels;
    LevelSpec levels_by_name_only;
    Logger* parent = nullptr;
    const char* name_ptr = nullptr;
    size_t name_len = 0;
    const unsigned depth = 0;
    unsigned version = 0;
    bool sensitive = false;
  };
} // namespace np::log

namespace np {
  using log::Logger;
}
#endif
