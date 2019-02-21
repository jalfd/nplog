add_subdirectory(external)

add_executable(example example.cc)
target_link_libraries(example PRIVATE nplog)

add_executable(tests
    tests.cc
    mocks.hpp
    test-utils.hpp
    macros-test.cc
    CommonTest.cc
    ScopedMessage-test.cc
    FormatterTest.cc
    SerializerTest.cc
    LogTest.cc
    ConfigTest.cc
)
target_link_libraries(tests PRIVATE Catch picojson nplog)

add_test(NAME tests COMMAND tests)
