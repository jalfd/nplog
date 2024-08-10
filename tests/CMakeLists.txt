add_executable(example example.cpp)
target_link_libraries(example PRIVATE nplog)

add_executable(tests
    macros.cpp
    common.cpp
    scopedmessage.cpp
    formatter.cpp
    serializer.cpp
    loggroup.cpp
    config.cpp
    messagebuffer.cpp
    loggroupprops.cpp
    bm-header.cc
    bm-params.cc
    bm-buffer.cc
)
target_link_libraries(tests PRIVATE picojson Catch2::Catch2WithMain nplog)

if (NPLOG_JUNIT_REPORTER)
    add_test(NAME tests COMMAND tests -r junit -o ${NPLOG_JUNIT_REPORTER})
else()
    add_test(NAME tests COMMAND tests)
endif()
