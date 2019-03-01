add_subdirectory(external)
add_executable(example example.cc)
target_link_libraries(example PRIVATE nplog)

add_executable(tests
    tests.cc
    test-utils.hpp
    macros-test.cc
    ScopedMessage-test.cc
    FormatterTest.cc
    SerializerTest.cc
    LogTest.cc
)
target_link_libraries(tests PRIVATE Catch picojson nplog)
add_test(NAME tests COMMAND tests)

add_executable(compile_benchmark time_nplog_build.cc)
target_link_libraries(compile_benchmark PRIVATE nplog)
target_compile_definitions(compile_benchmark PRIVATE "CYCLE_TYPES")
set_target_properties(compile_benchmark PROPERTIES EXCLUDE_FROM_ALL TRUE)
# To get build command: jq -r 'map(select(.file | endswith("time_nplog_build.cc"))) | .[] | .command' compile_commands.json
# to build: time the above

find_package(Boost)
add_executable(bm bm-main.cc bm-nplog.cc)
target_link_libraries(bm nonius nplog)
target_include_directories(bm PRIVATE "${Boost_INCLUDE_DIR}")
