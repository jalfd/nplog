add_executable(tests
    tests.cc
    test-utils.hpp
    macros-test.cc
    ScopedMessage-test.cc
)
target_link_libraries(tests PRIVATE Catch nplog)

add_test(NAME tests COMMAND tests)

include(ExternalProject)
ExternalProject_Add(
    CatchDownload
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/include/catch
    URL https://raw.githubusercontent.com/catchorg/Catch2/v2.5.0/single_include/catch2/catch.hpp
    URL_HASH SHA1=a9a332fa21e91a037455d81b9d6f88332305e5be
    DOWNLOAD_NAME catch.hpp
    DOWNLOAD_NO_EXTRACT true
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_INSTALL ON
) 

add_library(Catch INTERFACE)
add_dependencies(Catch CatchDownload)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/external/include)
