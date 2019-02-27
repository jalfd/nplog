#ifndef NP_LOG_MACROS_HPP
#define NP_LOG_MACROS_HPP

namespace np::log::internal {
    inline const char* getArgName(const char* name, const char*) { return name; }
    inline const char* getArgName(int, const char* name) { return name; }
}

#define ARG3(arg0, arg1, arg2) ((sm.suppressParam(arg0)) ? false : sm.addArg(arg1, arg2))
#define ARG2(arg0, arg1) ((sm.suppressParam(arg0)) ? false: sm.addArg(::np::log::internal::getArgName(arg0, #arg1), arg1))
#define ARG1(arg0) ((sm.suppressParam()) ? false: sm.addArg(#arg0, arg0))

#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1,_2,_3,NAME,...) NAME
#define ARG(...) INTERNAL_NP_VAR_MACRO_SELECTOR(__VA_ARGS__, ARG3, ARG2, ARG1)(__VA_ARGS__)

// rationale: making the macro explicit about which log object to use makes it testable gives the
// most freedom of use. Users can define a macro which hides this parameter by just fetching a
// (TLS-)global object.
#define LOG_IMPL(log, level, msg, ...) \
  if (!log.suppressMessage(level)) { \
    ::np::ScopedMessage sm(log, __FILE__, __LINE__, level, msg); \
    (void) __VA_ARGS__; \
  }

#define LOG(...) \
    LOG_IMPL(__VA_ARGS__, (void)nullptr)

#endif
