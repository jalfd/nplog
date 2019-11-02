#ifndef NP_LOG_MACROS_HPP
#define NP_LOG_MACROS_HPP

#include <nplog/common.hpp>

namespace np::log::internal {
  inline const char* getArgName(const char* name, const char*) { return name; }
  inline const char* getArgName(level_type, const char* name) { return name; }
} // namespace np::log::internal

#define NP_MSVC_EXPAND_INDIRECT(m, args) m args

#define NP_ARG3(arg0, arg1, arg2) ((sm.suppressParam(arg0)) ? false : sm.addArg(arg1, arg2))
#define NP_ARG2(arg0, arg1) \
  ((sm.suppressParam(arg0)) ? false : sm.addArg(::np::log::internal::getArgName(arg0, #arg1), arg1))
#define NP_ARG1(arg0) ((sm.suppressParam()) ? false : sm.addArg(#arg0, arg0))

#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1, _2, _3, NAME, ...) NAME
#define NP_ARG(...) \
  NP_MSVC_EXPAND_INDIRECT(INTERNAL_NP_VAR_MACRO_SELECTOR, (__VA_ARGS__, NP_ARG3, NP_ARG2, NP_ARG1)) \
  (__VA_ARGS__)

#define NP_LOG_IMPL(logger, msg_lvl, msg, ...) \
  if (auto lvl_threshold = logger.refreshLevels(logger.knownVersion()); \
      np::log::testLevel(msg_lvl, lvl_threshold.message)) { \
    ::np::log::ScopedMessage sm(logger, __FILE__, __LINE__, msg_lvl, msg, lvl_threshold.param); \
    (void) __VA_ARGS__; \
  }

#define NP_LOG(...) NP_MSVC_EXPAND_INDIRECT(NP_LOG_IMPL, (__VA_ARGS__, (void) nullptr))

#ifndef NP_LOG_REQUIRE_PREFIXES
#define LOG NP_LOG
#define ARG NP_ARG
#endif

#endif
