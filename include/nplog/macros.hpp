#ifndef NP_LOG_MACROS_HPP
#define NP_LOG_MACROS_HPP

#include <nplog/common.hpp>

namespace np::log::internal {
  inline const char* getParamName(const char* name, const char*) { return name; }
  inline const char* getParamName(level_type, const char* name) { return name; }
} // namespace np::log::internal

#define NP_MSVC_EXPAND_INDIRECT(m, args) m args

#define NP_WITH3(param0, param1, param2) \
  ((::np::log::suppressParam(lvl_threshold, msg_level, param0)) ? false \
                                                                : sm.addParam(param1, param2))
#define NP_WITH2(param0, param1) \
  ((::np::log::suppressParam(lvl_threshold, msg_level, param0)) \
      ? false \
      : sm.addParam(::np::log::internal::getParamName(param0, #param1), param1))
#define NP_WITH1(param0) \
  ((::np::log::suppressParam(lvl_threshold, msg_level)) ? false : sm.addParam(#param0, param0))

#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1, _2, _3, NAME, ...) NAME
#define NP_WITH(...) \
  NP_MSVC_EXPAND_INDIRECT( \
    INTERNAL_NP_VAR_MACRO_SELECTOR, (__VA_ARGS__, NP_WITH3, NP_WITH2, NP_WITH1)) \
  (__VA_ARGS__)

#define NP_LOG_IMPL(logger, msg_lvl, msg, ...) \
  if (auto lvl_threshold = logger.refreshLevels(logger.knownVersion()); \
      np::log::testLevel(msg_lvl, lvl_threshold.message)) { \
    ::np::log::ScopedMessage sm(logger, __FILE__, __LINE__, msg_lvl, msg); \
    const auto msg_level = msg_lvl; \
    (void) __VA_ARGS__; \
  }

#define NP_LOG(...) NP_MSVC_EXPAND_INDIRECT(NP_LOG_IMPL, (__VA_ARGS__, (void) nullptr))

#ifndef NP_LOG_REQUIRE_PREFIXES
#define LOG NP_LOG
#define WITH NP_WITH
#endif

#endif
