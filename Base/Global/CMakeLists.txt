# /*
# For more information, please see: http://software.sci.utah.edu
# 
# The MIT License
# 
# Copyright (c) 2018 Scientific Computing and Imaging Institute,
# University of Utah.
# 
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
# */


project(Global)

# this might not be needed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# This probably can be combined. SceneGraph_INCLUDES is defined in SceneGraph since 
# FLObject and SceneGraph depend on each other. Hopefully this will be fixed in the 
# future
include_directories(${SceneGraph_INCLUDES} ${FUI_INCLUDES})

# We manually set the Headers and Source files instead of Globbing them. This gives us
# better control of what goes in the project and what does not. 
set(HEADER_FILES Global.hpp)

set(SOURCES Global.cpp)

# FLObject includes and the library are cached internally since SceneGraph and 
# others depend on this library.
set(Global_INCLUDES ${HEADER_FILES} CACHE INTERNAL "")
set(Global_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")

# We create FLObject as a static library
add_library(Global STATIC ${HEADER_FILES} ${SOURCES}) 

# This library depends on Booost and Glew.
add_dependencies(Global SceneGraph)

# We link with SceneGraph and Boost
target_link_libraries(Global SceneGraph)

# I do not think this is needed.
install(TARGETS Global
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
