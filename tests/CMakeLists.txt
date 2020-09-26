add_subdirectory(external)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE nplog)

add_executable(tests
    tests.cpp
    macros.cpp
    common.cpp
    scopedmessage.cpp
    formatter.cpp
    serializer.cpp
    log.cpp
    config.cpp
    messagebuffer.cpp
    logparams.cpp
)
target_link_libraries(tests PRIVATE Catch picojson nplog)

if (NPLOG_JUNIT_REPORTER)
    add_test(NAME tests COMMAND tests -r junit -o ${NPLOG_JUNIT_REPORTER})
else()
    add_test(NAME tests COMMAND tests)
endif()
