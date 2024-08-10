#ifndef NP_LOG_LOGGROUP_HPP
#define NP_LOG_LOGGROUP_HPP

#include <nplog/export.hpp>
#include <nplog/formatter.hpp>
#include <nplog/common.hpp>
#include <string_view>

namespace np::log {
  struct MessageBuffer;
  struct Serializer;
  struct ValueSerializer;

  struct NPLOG_EXPORT LogGroup {
    explicit LogGroup(LogGroup* parent = nullptr, const char* name = nullptr) noexcept;
    explicit LogGroup(const char* name) noexcept;

    LogGroup(const LogGroup&) = delete;

    ~LogGroup() noexcept;

    LevelSpec refreshLevels(unsigned version, unsigned global_version, bool exclude_depth = false) noexcept;

    unsigned knownVersion() const noexcept { return version; }

    std::string_view name() const noexcept { return std::string_view(name_ptr, name_len); }

  private:
    LevelSpec effective_levels;
    LevelSpec levels_by_name_only;
    LogGroup* parent = nullptr;
    const char* name_ptr = nullptr;
    size_t name_len = 0;
    const unsigned depth = 0;
    unsigned version = 0;
  };
} // namespace np::log

namespace np {
  using log::LogGroup;
}
#endif
