#This is an initial CMake configuration file for FluoRender
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
add_library(Glew glew.c glewinfo.c visualinfo.c)
