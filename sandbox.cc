#include <string> // to_string
#include <iostream>
#include <vector>

// NP_INFO(log, "hello", ARG(foo));

#define INTERNAL_NP_UNIQUE_NAME_LINE2(name, line) name##line
#define INTERNAL_NP_UNIQUE_NAME_LINE(name, line) INTERNAL_NP_UNIQUE_NAME_LINE2(name, line)

#define NP_DEBUG ScopedMessage(__FILE__, __LINE__, ::np::Severity::Debug)
#define NP_INFO ScopedMessage(__FILE__, __LINE__, ::np::Severity::Info)
#define NP_WARNING ScopedMessage(__FILE__, __LINE__, ::np::Severity::Warning)
#define NP_ERROR ScopedMessage(__FILE__, __LINE__, ::np::Severity::Error)
#define ARG(msg) std::pair<const char*, const decltype(msg)&>(#msg, msg)
#define ARG1(msg) std::pair<const char*, const decltype(msg)&>(#msg, msg)
#define ARG2(lhs, rhs) std::pair<const char*, const decltype(rhs)&>(lhs, rhs)
//#define ARG3(unused, lhs, rhs) std::pair<const char*, const decltype(msg)&>(#msg, msg)

//#define INTERNAL_NP_VAR_MACRO_SELECTOR(_1,_2,_3,NAME,...) NAME
//#define ARG(...) INTERNAL_NP_VAR_MACRO_SELECTOR(__VA_ARGS__, ARG3, ARG2, ARG1)(__VA_ARGS__)

namespace np {
    enum class Severity {
        Debug,
        Info,
        Warning,
        Error
    };

    const char* stringify(Severity s) {
        switch (s) {
            case Severity::Debug:
                return "Debug  ";
            case Severity::Info:
                return "Info   ";
            case Severity::Warning:
                return "Warning";
            case Severity::Error:
                return "Error  ";
        }
    }
}
struct Log {};

thread_local Log default_log;

struct ScopedMessage {
    ScopedMessage(const char* file, int line, np::Severity severity){
        header = '[' + std::string(file) + ':' + std::to_string(line) +"][" + np::stringify(severity) + ']';
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
    void serializeArg(const std::pair<const char*, const T&> arg) {
        args.push_back(std::string(arg.first) + " := " + std::to_string(arg.second));
    }

    void applyArgs() {
    }
    template <typename Head, typename ...Tail>
    void applyArgs(const std::pair<const char*, const Head&> head, Tail... tail) {
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
    NP_WARNING("hello2", ARG(2+2));
    NP_ERROR("hello3", ARG(2+2), ARG2("foo", 42));
}
