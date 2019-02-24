#include <nplog/macros.hpp>
#include <nplog/Log.hpp>
#include <nplog/ScopedMessage.hpp>
#include <nplog/Formatter.hpp>

np::Log log;

int main() {
    np::Log::setSink(np::getStdErrSink());
    int fortytwo = 42;
    LOG(log, 0, "logging a message", ARG(fortytwo));
    LOG(log, 0, "logging a message");
    LOG(log, 0, "logging a message", ARG(8, fortytwo));
    LOG(log, 0, "logging a message", ARG("myval", fortytwo));
}
