#This is an initial CMake configuration file for FluoRender
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
file(GLOB srcs *.c)
file(GLOB hdrs1 *.h)
file(GLOB hdrs2 GL/*.h)
add_library(Glew ${srcs} ${hdrs1} ${hdrs2})
