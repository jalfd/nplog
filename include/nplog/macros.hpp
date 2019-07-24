#ifndef NP_LOG_MACROS_HPP
#define NP_LOG_MACROS_HPP

#include <nplog/common.hpp>

namespace np::log::internal {
  inline const char* getArgName(const char* name, const char*) { return name; }
  inline const char* getArgName(level_type, const char* name) { return name; }
} // namespace np::log::internal

#define NP_MSVC_EXPAND_INDIRECT(m, args) m args

#define ARG3(arg0, arg1, arg2) ((sm.suppressParam(arg0)) ? false : sm.addArg(arg1, arg2))
#define ARG2(arg0, arg1) \
  ((sm.suppressParam(arg0)) ? false : sm.addArg(::np::log::internal::getArgName(arg0, #arg1), arg1))
#define ARG1(arg0) ((sm.suppressParam()) ? false : sm.addArg(#arg0, arg0))

#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1, _2, _3, NAME, ...) NAME
#define ARG(...) \
  NP_MSVC_EXPAND_INDIRECT(INTERNAL_NP_VAR_MACRO_SELECTOR, (__VA_ARGS__, ARG3, ARG2, ARG1)) \
  (__VA_ARGS__)

#define LOG_IMPL(log, msg_lvl, msg, ...) \
  if (auto lvl_threshold = log.refreshLevels(log.knownVersion()); \
      np::testLevel(msg_lvl, lvl_threshold.message)) { \
    ::np::ScopedMessage sm(log, __FILE__, __LINE__, msg_lvl, msg, lvl_threshold.param); \
    (void) __VA_ARGS__; \
  }

#define LOG(...) NP_MSVC_EXPAND_INDIRECT(LOG_IMPL, (__VA_ARGS__, (void) nullptr))

#endif
