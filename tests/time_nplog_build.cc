#include <nplog/macros.hpp>
#include <nplog/Log.hpp>
#include <nplog/ScopedMessage.hpp>
#include <nplog/Formatter.hpp>

#include "benchcompile_framework.hpp"

// Build command:
// time clang++ -Itests -I../tests -Itests/external/include -I../include -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk -std=c++1z -DCYCLE_TYPES -o /dev/null -c ../tests/time_nplog_build.cc


template <int N>
struct ArgType {};

#define PARAMETER_TYPE(N) ArgType<N>

#define STATIC_SETUP_ONCE() \
np::Log log;

#define STATIC_SETUP_PER_INSTANCE(Type) \
template <> \
struct np::Formatter<Type> { \
    void operator()(Type&& val, Serializer& srl){ \
        srl.write(0); \
    } \
};

#define TEST_IMPL(Type) \
    Type arg; \
    LOG(log, 0, "hello world", ARG(arg));

INSTANTIATE_BENCHMARK()
