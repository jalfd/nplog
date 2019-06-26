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

add_library(Date INTERFACE)
target_include_directories(Date INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/external/include)

add_library(nplog SHARED
    src/Log.cc
    src/Serializer.cc
    src/Formatter.cc
    include/nplog/Formatter.hpp
    include/nplog/Log.hpp
    include/nplog/Serializer.hpp
    include/nplog/ScopedMessage.hpp
    include/nplog/macros.hpp
)

target_link_libraries(nplog Date)
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

install(TARGETS nplog DESTINATION lib)
install(FILES ${nplog_public_headers} ${CMAKE_BINARY_DIR}/include/nplog/export.hpp DESTINATION include/nplog)
