#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <type_traits>

// TODO
// Convert internal string to vector
// store args jsonified
// extend logStringify with a bool& json_literal

// Just for testing. In real usage, including FMT is up to the user
#define FMT_HEADER_ONLY
#include "fmt/include/fmt/core.h"
#include "fmt/include/fmt/format.h"

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
namespace np {
    namespace internal {
        auto locale = []() {
            static struct ScopedLoc {
#ifdef _WIN32
                using locale_t = _locale_t;
#endif
                ScopedLoc() :
#ifdef _WIN32
                    _create_locale(LC_ALL, "C");
#else
                loc(newlocale(LC_ALL_MASK, "C", 0))
#endif
                {}

                ~ScopedLoc()
                {
#ifdef _WIN32
                    _free_locale(loc);
#else
                    freelocale(loc);
#endif
                }


                locale_t loc;
            } locale;
            return locale;
        }();
#ifdef _WIN32
        using locale_t = _locale_t;
        locale_t _create_locale(LC_ALL, "C");
#endif
    }
    using str_out_iter = std::back_insert_iterator<std::string>; // FIXME: should be vector

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

    template <typename T>
    auto logStringify(str_out_iter it, const T& val) -> typename std::enable_if<!std::is_floating_point<T>::value, str_out_iter>::type
    {
//    str_out_iter logStringify(str_out_iter it, const T& val) {
#ifdef FMT_VERSION
        return fmt::format_to(it, "{}", val);
#else
        /*
        struct np_osstream : std::ostringstream {

            public:
        };
        std::ostringstream strm;
        strm << val;
        return std::copy(strm.rdbuf()->pbase, strm.rdbuf()->pptr, it);
        */

        const auto str = (std::ostringstream() << val).str();
        return std::copy(str.begin(), str.end(), it);
#endif
    }

    template <typename T>
        auto logStringify(str_out_iter it, const T& val) -> typename std::enable_if<std::is_floating_point<T>::value, str_out_iter>::type
{
    char buf[32];
    //const auto len = snprintf_l(buf, 32, internal::locale.loc, "%f", val);
    const auto len = snprintf(buf, 32, "%f", val);
//    const auto len = _snprintf_l(buf, 32, "%f", val, internal::locale.loc); // WIN32
    return std::copy_n(buf, len, it);
}



    str_out_iter logStringify(str_out_iter it, Severity s) {
//    const char* logStringify(Severity s) {
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
        if (std::get<0>(arg) >= np::min_severity) {
            std::string tmp;
            np::logStringify(std::back_inserter(tmp), std::get<2>(arg));
            args.push_back(std::string(std::get<1>(arg)) + " := " + tmp);
        }
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
};

int main() {
    NP_INFO(Log(), "hello1");
    int foo = 42;
    NP_WARNING("hello2", ARG(2+2));
    NP_ERROR("hello3", ARG(foo), ARG("foo", 42), ARG(np::Severity::Debug, std::sqrt(42)));
    NP_ERROR("hello4", ARG(2+2), ARG("foo", 42), ARG(np::Severity::All, std::sqrt(42)));

    NP_INFO(Log(), "hello1", ARG(10000.1));
}
