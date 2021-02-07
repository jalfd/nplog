#ifndef NP_LOGGROUPPROPS_HPP
#define NP_LOGGROUPPROPS_HPP

#include <nplog/loggroup.hpp>
#include "messagebuffer.hpp"
namespace np::log {
  struct LogGroupProps {
    LogGroupProps() noexcept = default;
    LogGroupProps(LogGroupProps* parent, std::initializer_list<LogProp> props) noexcept;

    MessageBuffer data;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>
      offsets; // FIXME: put in a struct, first, name last, last
  };
}
#endif
