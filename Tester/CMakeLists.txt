project(Test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#find_package(FLObject)

include_directories(${CMAKE_SOURCE_DIR}/Base/FLObject ${OPENGL_INCLUDE_DIRS})
#include_directories(${FLObject_INCLUDES})

link_directories(${FLObject_LIBRARY_DIR})

add_executable(Test main.cpp objecttest.cpp asserts.hpp tests.hpp)
add_dependencies(Test FLObject Boost_external_download FFmpeg_external_download)
target_link_libraries(Test 
  FLObject 
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${FFmpeg_LIBRARIES}
)
