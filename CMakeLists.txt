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

add_library(Date INTERFACE)
target_include_directories(Date INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/external/include)

set (nplog_public_headers
    include/nplog/Formatter.hpp
    include/nplog/Log.hpp
    include/nplog/Serializer.hpp
    include/nplog/ScopedMessage.hpp
    include/nplog/macros.hpp
    )
add_library(nplog SHARED
    src/Log.cc
    src/Serializer.cc
    src/Formatter.cc
    ${nplog_public_headers}
)

target_link_libraries(nplog Date)
target_include_directories(nplog
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(nplog
EXPORT_FILE_NAME
include/nplog/export.hpp
)

# For exporting target "the nice way",
# see https://cmake.org/cmake/help/latest/command/install.html?highlight=install#export
# and https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/

add_subdirectory(tests)

install(TARGETS nplog DESTINATION lib)
install(FILES ${nplog_public_headers} ${CMAKE_BINARY_DIR}/include/nplog/export.hpp DESTINATION include/nplog)
