#ifndef NP_LOG_LOGGROUP_HPP
#define NP_LOG_LOGGROUP_HPP

#include <nplog/config.hpp>
#include <nplog/export.hpp>
#include <nplog/formatter.hpp>
#include <nplog/messagebuffer.hpp>
#include <nplog/serializer.hpp>

#include <string_view>

namespace np::log {
  struct LogParam { // FIXME: move to detail
    template <typename T>
    LogParam(const char* name, T&& value)
      : name(name), func([=](Serializer& serializer, MessageBuffer& buffer) {
        serializer.writeKey(name);
        const auto name_end = buffer.messageSize(); // FIXME: do we really want to depend on buffer?
        auto vs = serializer.valueSerializer();
        np::log::format(value, vs);
        return static_cast<uint32_t>(name_end);
      }) {}
    const char* name;
    std::function<uint32_t(Serializer&, MessageBuffer&)> func;
  };
  struct LogGroupProps {
    LogGroupProps() = default;
    LogGroupProps(LogGroupProps* parent, std::initializer_list<LogParam> params);

    MessageBuffer data;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>
      offsets; // FIXME: put in a struct, first, name last, last
  };

  struct NPLOG_EXPORT LogGroup {
    using buffer_type = MessageBuffer;
    using serializer_type = Serializer;

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
