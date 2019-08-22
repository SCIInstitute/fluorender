project(Test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#find_package(FLObject)

#include_directories(${CMAKE_SOURCE_DIR}/Base/FLObject)
include_directories(${FLObject_INCLUDES})

add_executable(Test main.cpp objecttest.cpp asserts.hpp tests.hpp)
add_dependencies(Test FLObject)
target_link_libraries(Test 
  FLObject 
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${FFmpeg_LIBRARIES}
)
