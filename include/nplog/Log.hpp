#ifndef NP_LOG_LOG_HPP
#define NP_LOG_LOG_HPP

#include <nplog/Config.hpp>
#include <nplog/Formatter.hpp>
#include <nplog/Serializer.hpp>
#include <nplog/export.hpp>

#include <string_view>
#include <vector>

namespace np {
  struct NPLOG_EXPORT Log {
    using buffer_type = std::vector<char>;
    using serializer_type = Serializer;

    explicit Log(Log* parent = nullptr, const char* name = nullptr);
    explicit Log(const char* name);
    Log(const Log&) = delete;

    buffer_type acquireBuffer();

    void submitMessage(level_type level, buffer_type& buffer);

    void releaseBuffer(buffer_type&& buf);

    Levels refreshLevels(unsigned version, bool exclude_depth = false);

    unsigned knownVersion() const { return version; }

    std::string_view name() const { return std::string_view(name_ptr, name_len); }

  private:
    Levels effective_levels;
    Levels levels_by_name_only;
    Log* parent = nullptr;
    const char* name_ptr = nullptr;
    size_t name_len = 0;
    const unsigned depth = 0;
    unsigned version = 0;
  };

  NPLOG_EXPORT std::function<void(level_type, std::string_view)> getStdErrSink();
} // namespace np
#endif
