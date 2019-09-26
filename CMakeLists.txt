cmake_minimum_required(VERSION 3.10)
enable_testing()

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

if (NOT WIN32)
    add_definitions(-Wfatal-errors)
endif()

add_library(Date INTERFACE IMPORTED)

# Requires CMake 3.11, so use the more verbose option instead
#target_include_directories(Date INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/external/include)
set_property(TARGET Date APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/src/external/include)

find_package(Threads REQUIRED)

add_library(nplog
    src/log.cpp
    src/serializer.cpp
    src/formatter.cpp
    src/config.cpp
    src/configimpl.hpp
    src/configimpl.cpp
    src/utility.hpp
    src/platform.hpp
    src/platform.cpp
    include/nplog/formatter.hpp
    include/nplog/log.hpp
    include/nplog/serializer.hpp
    include/nplog/scopedmessage.hpp
    include/nplog/macros.hpp
    include/nplog/config.hpp
    )

add_library(np::log ALIAS nplog)

target_link_libraries(nplog PRIVATE Date PUBLIC Threads::Threads)
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
)

include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(nplog
    EXPORT_FILE_NAME
    include/nplog/export.hpp
)

add_subdirectory(tests)

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
