cmake_minimum_required(VERSION 3.27)
enable_testing()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

project(nplog LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
option(BUILD_SHARED_LIBS "Build as shared library" ON)
option(NP_CHECK_CONVERSIONS "Check for overflows in various internal conversions" OFF)
option(NP_LIBRARY_DEV "Enable compiler flags useful to nplog project developers" OFF)

find_package(Threads REQUIRED)
find_package(Catch2 REQUIRED)
find_package(date CONFIG REQUIRED)
find_path(PICOJSON_INCLUDE_DIRS "picojson/picojson.h")

# Wrap up picojson in an ad-hoc imported target
add_library(picojson INTERFACE IMPORTED)
target_include_directories(picojson INTERFACE ${PICOJSON_INCLUDE_DIRS})

if (NP_LIBRARY_DEV)
    if (NOT WIN32)
        add_definitions(-Wfatal-errors)
    endif()
endif()

if (NOT WIN32) #FIXME
add_definitions(-gdwarf-4)
endif()

add_library(nplog
    src/loggroup.cpp
    src/serializer.cpp
    src/scopedmessage.cpp
    src/formatter.cpp
    src/messagebuffer.cpp
    src/common.cpp
    src/config.cpp
    src/configimpl.hpp
    src/configimpl.cpp
    src/utility.hpp
    src/platform.hpp
    src/platform.cpp
    include/nplog/formatter.hpp
    include/nplog/loggroup.hpp
    include/nplog/serializer.hpp
    include/nplog/scopedmessage.hpp
    include/nplog/macros.hpp
    include/nplog/config.hpp
    )

add_library(np::log ALIAS nplog)

target_link_libraries(nplog PRIVATE date::date PUBLIC Threads::Threads)
if (WIN32)
    target_link_libraries(nplog PRIVATE ws2_32)
endif()
target_include_directories(nplog
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(nplog PROPERTIES
    EXPORT_NAME log
    DEBUG_POSTFIX d
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(nplog
    EXPORT_FILE_NAME
    include/nplog/export.hpp
)

if (NP_CHECK_CONVERSIONS)
    target_compile_options(nplog PRIVATE -DNP_CHECK_CONVERSIONS=1)
endif()

target_compile_options(nplog PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Werror -pedantic-errors -Wall -Wextra -Wconversion -O3>
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4 /WX /Ob2 /Zo>)

add_subdirectory(tests)

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") AND NPLOG_LLVM_COVERAGE)
    find_program(LLVM_PROFDATA
        "llvm-profdata"
        PATHS ${NPLOG_LLVM_BIN_DIR}
    )
    find_program(LLVM_COV
        "llvm-cov"
        PATHS ${NPLOG_LLVM_BIN_DIR}
    )

    target_compile_options(nplog PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(nplog PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_compile_options(tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_compile_options(example PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(example PRIVATE -fprofile-instr-generate -fcoverage-mapping)

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=tests.profraw $<TARGET_FILE:tests>
        COMMAND ${LLVM_PROFDATA} merge -sparse tests.profraw -o tests.profdata
        COMMAND ${LLVM_COV} show -format=html -output-dir=coverage -instr-profile tests.profdata $<TARGET_FILE:nplog>
        COMMAND ${CMAKE_COMMAND} -E echo Coverage report available in file://${CMAKE_BINARY_DIR}/coverage/index.html
        DEPENDS tests
        COMMENT "Generating coverage report"
        VERBATIM)
endif()

install(TARGETS nplog
    EXPORT nplog-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
    )
install(FILES ${CMAKE_BINARY_DIR}/include/nplog/export.hpp DESTINATION include/nplog)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT nplog-targets
    FILE nplog-targets.cmake
    NAMESPACE np::
    DESTINATION lib/cmake/nplog
    )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/nplog-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/nplog-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/nplog-config.cmake
    INSTALL_DESTINATION lib/cmake/nplog
    )

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/nplog-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/nplog-config-version.cmake
    DESTINATION lib/cmake/nplog
    )

export(EXPORT nplog-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/nplog-targets.cmake NAMESPACE np::)
export(PACKAGE nplog)
