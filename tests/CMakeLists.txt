add_executable(example example.cpp)
target_link_libraries(example PRIVATE nplog)

add_executable(tests
    contexttracker.cpp
    macros.cpp
    common.cpp
    scopedmessage.cpp
    formatter.cpp
    serializer.cpp
    loggroup.cpp
    config.cpp
    messagebuffer.cpp
    loggroupprops.cpp
    bm-header.cpp
    bm-params.cpp
    bm-buffer.cpp
    bm-nplog.cpp
)
target_link_libraries(tests PRIVATE picojson Catch2::Catch2WithMain nplog)
if (WIN32)
set (nullfile "\"NUL\"")
else()
set (nullfile "\"/dev/null\"")
endif()
target_compile_definitions(tests PRIVATE "-DNULLFILE=${nullfile}")

if (NPLOG_JUNIT_REPORTER)
    add_test(NAME tests COMMAND tests -r junit -o ${NPLOG_JUNIT_REPORTER})
else()
    add_test(NAME tests COMMAND tests)
endif()
