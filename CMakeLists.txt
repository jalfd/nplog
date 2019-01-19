cmake_minimum_required(VERSION 3.10)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
enable_testing()

project(nplog LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(nplog SHARED
    include/nplog/Log.hpp
    src/Log.cc
    src/Serializer.cc
    include/nplog/Formatter.hpp
    include/nplog/Log.hpp
    include/nplog/Serializer.hpp
    include/nplog/ScopedMessage.hpp
    include/nplog/macros.hpp
)

target_include_directories(nplog
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(tests)
