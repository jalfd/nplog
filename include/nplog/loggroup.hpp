#ifndef NP_LOG_LOGGROUP_HPP
#define NP_LOG_LOGGROUP_HPP

#include <nplog/export.hpp>
#include <nplog/formatter.hpp>
#include <nplog/serializer.hpp>

#include <string_view>

namespace np::log {
  struct MessageBuffer;
  struct LogGroupProps;

  struct LogParam { // FIXME: move to detail
    template <typename T>
    LogParam(const char* name, const T& value)
      : name(name), value(&value), format_func([](const void* value, ValueSerializer& vs) {
        np::log::format(*reinterpret_cast<const T*>(value), vs);
      }) {}
    const char* name;
    const void* value;

    void(*format_func)(const void*, ValueSerializer&);

    static uint32_t serialize(Serializer& serializer, MessageBuffer& buffer, const char* name, const void* value, void(*format_func)(const void*, ValueSerializer&));
  };

  struct NPLOG_EXPORT LogGroup {
    explicit LogGroup(LogGroup* parent = nullptr, const char* name = nullptr);
    explicit LogGroup(const char* name);

    explicit LogGroup(LogGroup* parent, const char* name, std::initializer_list<LogParam> params);

    LogGroup(const LogGroup&) = delete;

    ~LogGroup();

    LevelSpec refreshLevels(unsigned version, unsigned global_version, bool exclude_depth = false);

    unsigned knownVersion() const { return version; }

    std::string_view name() const { return std::string_view(name_ptr, name_len); }

    LogGroupProps* props() { return group_props; } // FIXME: access hack
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
