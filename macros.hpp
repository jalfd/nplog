#ifndef NP_LOG_MACROS_HPP
#define NP_LOG_MACROS_HPP

#define ARG3(arg0, arg1, arg2) ((sm.testArg(arg0)) ? sm.serialize(arg1, arg2) : false)
#define ARG2(arg0, arg1) ((sm.testArg(arg0)) ? sm.serialize(sm.getName(arg0, #arg1), arg1) : false)
#define ARG1(arg0) ((sm.testArg()) ? sm.serialize(#arg0, arg0) : false)

#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1,_2,_3,NAME,...) NAME
#define ARG(...) INTERNAL_NP_VAR_MACRO_SELECTOR(__VA_ARGS__, ARG3, ARG2, ARG1)(__VA_ARGS__)

// rationale: making the macro explicit about which log object to use makes it testable gives the
// most freedom of use. Users can define a macro which hides this parameter by just fetching a
// (TLS-)global object.
#define LOG_IMPL(log, level, msg, ...) \
  if (log.testMessage(level)) { \
    ::np::ScopedMessage sm(msg); \
    (void) __VA_ARGS__; \
  }

#define LOG(...) \
    LOG_IMPL(__VA_ARGS__, (void)nullptr)

#endif
