#ifndef NP_LOG_LOGGROUP_HPP
#define NP_LOG_LOGGROUP_HPP

#include <nplog/export.hpp>
#include <nplog/formatter.hpp>
#include <nplog/common.hpp>
#include <string_view>

namespace np::log {
  struct MessageBuffer;
  struct LogGroupProps;
  struct Serializer;
  struct ValueSerializer;

  struct LogProp { // FIXME: move to detail
    template <typename T>
    LogProp(const char* name, const T& value) noexcept
      : name(name), value(&value), format_func([](const void* value, ValueSerializer& vs) noexcept {
        np::log::format(*reinterpret_cast<const T*>(value), vs);
      }) {}
    const char* name;
    const void* value;

    void(*format_func)(const void*, ValueSerializer&) noexcept;

    static uint32_t serialize(Serializer& serializer, MessageBuffer& buffer, const char* name, const void* value, void(*format_func)(const void*, ValueSerializer&)) noexcept;
  };

  struct NPLOG_EXPORT LogGroup {
    explicit LogGroup(LogGroup* parent = nullptr, const char* name = nullptr) noexcept;
    explicit LogGroup(const char* name) noexcept;

    explicit LogGroup(LogGroup* parent, const char* name, std::initializer_list<LogProp> props) noexcept;

    LogGroup(const LogGroup&) = delete;

    ~LogGroup() noexcept;

    LevelSpec refreshLevels(unsigned version, unsigned global_version, bool exclude_depth = false) noexcept;

    unsigned knownVersion() const noexcept { return version; }

    std::string_view name() const noexcept { return std::string_view(name_ptr, name_len); }

    LogGroupProps* props() const noexcept { return group_props; } // FIXME: access hack
  private:
    LevelSpec effective_levels;
    LevelSpec levels_by_name_only;
    LogGroup* parent = nullptr;
    LogGroupProps* group_props = nullptr;
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
