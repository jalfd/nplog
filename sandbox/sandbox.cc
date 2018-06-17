#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <type_traits>

// TODO
// Convert internal string to vector
// store args jsonified
// extend logStringify with a bool& json_literal

#include "iterator.hpp"

// Just for testing. In real usage, including FMT is up to the user
#define FMT_HEADER_ONLY
#include "../fmt/include/fmt/core.h"
#include "../fmt/include/fmt/format.h"

#define INTERNAL_NP_STRINGIFY2(x) #x
#define INTERNAL_NP_STRINGIFY(x) INTERNAL_NP_STRINGIFY2(x)
#define INTERNAL_NP_UNIQUE_NAME_LINE2(name, line) name##line
#define INTERNAL_NP_UNIQUE_NAME_LINE(name, line) INTERNAL_NP_UNIQUE_NAME_LINE2(name, line)

#define NP_DEBUG ScopedMessage(__FILE__, np::arraySize(__FILE__), INTERNAL_NP_STRINGIFY(__LINE__), np::arraySize(INTERNAL_NP_STRINGIFY(__LINE__)), ::np::Severity::Debug)
#define NP_INFO ScopedMessage(__FILE__, np::arraySize(__FILE__), INTERNAL_NP_STRINGIFY(__LINE__), np::arraySize(INTERNAL_NP_STRINGIFY(__LINE__)), ::np::Severity::Info)
#define NP_WARNING ScopedMessage(__FILE__, np::arraySize(__FILE__), INTERNAL_NP_STRINGIFY(__LINE__), np::arraySize(INTERNAL_NP_STRINGIFY(__LINE__)), ::np::Severity::Warning)
#define NP_ERROR ScopedMessage(__FILE__, np::arraySize(__FILE__), INTERNAL_NP_STRINGIFY(__LINE__), np::arraySize(INTERNAL_NP_STRINGIFY(__LINE__)), ::np::Severity::Error)

#define ARG1(arg0) ::np::expandArg(#arg0, arg0)
#define ARG2(arg0, arg1) ::np::expandArg(arg0, #arg1, arg1)
#define ARG3(arg0, arg1, arg2) ::np::expandArg(arg0, arg1, #arg2, arg2)

#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1,_2,_3,NAME,...) NAME
#define ARG(...) INTERNAL_NP_VAR_MACRO_SELECTOR(__VA_ARGS__, ARG3, ARG2, ARG1)(__VA_ARGS__)

#ifdef _WIN32
#include <locale.h>
#else
#include <xlocale.h>
#endif
#include <stdio.h> // for snprintf_l

#include "formatters.hpp"
namespace np {
    using str_out_iter = std::back_insert_iterator<std::vector<char>>;

    template <typename T, size_t N>
    constexpr inline size_t arraySize(const T (&)[N]) { return N; }

    enum class Severity {
        Never,
        Debug,
        Info,
        Warning,
        Error,
        All
    };
    template <typename OutIter>
    OutIter logStringify(OutIter it, Severity s) {
        const char (&str)[8] = [s]() -> const char(&)[8] {
            switch (s) {
                case Severity::Never:
                    return "       ";
                case Severity::Debug:
                    return "Debug  ";
                case Severity::Info:
                    return "Info   ";
                case Severity::Warning:
                    return "Warning";
                case Severity::Error:
                    return "Error  ";
                case Severity::All:
                    return "       ";
            }
        }();

        return std::copy_n(str, 8, it);
    }

static const Severity min_severity = Severity::Info;

template <typename T>
using ArgType = std::tuple<Severity, const char*, const T&>;

template <typename T>
auto expandArg(Severity s, const char* name, const T& val) {
    return ArgType<T>(s, name, val);;
}

template <typename T>
auto expandArg(Severity s, const char* name, const char *, const T& val) {
    return expandArg(s, name, val);
}

template <typename T>
auto expandArg(const char* name, const char *, const T& val) {
    return expandArg<T>(Severity::All, name, val);
}

template <typename T>
auto expandArg(const char* name, const T& val) {
    return expandArg(Severity::All, name, val);
}
}
struct Log {};

thread_local Log default_log;

struct ScopedMessage {
    ScopedMessage(const char* file, size_t filelen, const char* line, size_t linelen, np::Severity severity){
        auto it = std::back_inserter(header);
        *it++ = '[';
        it = std::copy_n(file, filelen - 1, it);
        *it++ = ':';
        it = std::copy_n(line, linelen - 1, it);
        *it++ = ']';
        *it++ = '[';
        it = np::logStringify(it, severity);
        *it++ = ']';
    }
    ~ScopedMessage() {
        std::cout << header << ": " << literal;
        if (!args.empty()) {
            std::cout << " [";
            bool first = true;
            for (const auto &arg : args) {
                if (!first) {
                    std::cout << ", ";
                }
                std::cout << arg;
                first = false;
            }
            std::cout << " ]";
        }
        std::cout << std::endl;
    }

    template <typename T>
    void serializeArg(const np::ArgType<T> arg) {
        using iter_t = np::JsonOutIterator<std::back_insert_iterator<std::vector<char>>>;

        if (std::get<0>(arg) < np::min_severity) {
            return;
        }

        {
            auto it = std::back_inserter(buffer);
            *it++ = '"';
        }
        np::logStringify(iter_t(std::back_inserter(buffer)), std::get<1>(arg));
        {
            auto it = std::back_inserter(buffer);
            *it++ = '"';
            *it++ = ':';
        }
        // FIXME: we need to figure out whether to quote this
        // I guess this is where traits come in
        np::logStringify(iter_t(std::back_inserter(buffer)), std::get<2>(arg));
        // FIXME: also need to figure out trailing comma

        // non-JSON variant:
        // args.push_back(std::string(std::get<1>(arg)) + " := " + tmp);
    }

    void applyArgs() {}

    template <typename Head, typename ...Tail>
    void applyArgs(const np::ArgType<Head> head, Tail... tail) {
        serializeArg(head);
        applyArgs(tail...);
    }

    template <size_t LiteralLen, typename ...Args>
    ScopedMessage &operator()(Log, const char (&str)[LiteralLen],  Args... args)
    {
        literal = str;
        applyArgs(args...);
        return *this;
    }
    template <size_t LiteralLen, typename ...Args>
    ScopedMessage &operator()(const char (&str)[LiteralLen], Args... args)
    {
        return (*this)(default_log, str, args...);
    }

    std::string header;
    const char* literal;
    std::vector<std::string> args;
    std::vector<char> buffer;
};

int main() {
    NP_INFO(Log(), "hello1");
    NP_WARNING("hello2", ARG(2+2));
    NP_ERROR("hello3", ARG("foo", 42), ARG(np::Severity::Debug, std::sqrt(42)));
    NP_ERROR("hello4", ARG("foo", 42), ARG(np::Severity::All, std::sqrt(42)));
    int bar = 99;
    NP_INFO("hello5", ARG(bar));
}

namespace { // FIXME: ad hoc stuff I haven't decided where to put
    struct Header
    {
        np::Severity severity;
        const std::string_view<char> file;
        const std::string_view<char> line;
    };

    using ArgStr = std::pair<std::string_view<char>, std::string<view>>;
    void write_backend(Header h, const std::string_view<char> &message, const ArgStr* first_arg, const ArgStr* last_arg);

struct state {
    std::vector<char> buffer;
    // some kind of effective log level?
};

    template <typename Traits, typename ...Args>
    void write(Header h, const std::string_view<char> &message, Args... args) // should be a member of log, or take a state arg, so we can have a buffer
    {
        // ok, now how do we format stuff in a generic way?
        // Do we take in a trait which tells us how to format?

        // stringify every argument, then pass to a backend.write(header, message, first_arg, last_arg);
        write_backend(h, stringify(message), stringify(args)...);
    }
}
