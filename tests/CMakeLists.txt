add_subdirectory(external)

add_executable(tests
    tests.cc
    mocks.hpp
    test-utils.hpp
    macros-test.cc
    ScopedMessage-test.cc
    FormatterTest.cc
    SerializerTest.cc
    LogTest.cc
    ConfigTest.cc
)
target_link_libraries(tests PRIVATE Catch picojson nplog)

add_test(NAME tests COMMAND tests)
