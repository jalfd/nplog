add_library(picojson INTERFACE)
target_include_directories(picojson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
