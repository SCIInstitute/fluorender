project(Test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#find_package(FLObject)

include_directories(${CMAKE_SOURCE_DIR}/Base/FLObject)

add_executable(Test main.cpp objecttest.cpp asserts.hpp tests.hpp)
add_dependencies(Test FLObject)
target_link_libraries(Test FLObject)
