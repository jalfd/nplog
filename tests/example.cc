#include <nplog/macros.hpp>
#include <nplog/Log.hpp>
#include <nplog/ScopedMessage.hpp>
#include <nplog/Formatter.hpp>

np::Log log;

int main() {
    int fortytwo = 42;
    LOG(log, 0, "logging a message", ARG(fortytwo));
    LOG(log, 0, "logging a message");
}
