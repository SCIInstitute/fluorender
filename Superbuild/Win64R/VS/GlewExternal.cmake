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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# The latest glew is taken from sourceforge. 
set(Glew_url "https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.zip/download")
set(Glew_Hash "MD5=dff2939fd404d054c1036cc0409d19f1")

# The source subdirectory must be set since glew has their cmake file burried
# in their directory structure.
ExternalProject_Add(glew_external_download
	URL ${Glew_url}
	URL_HASH ${Glew_Hash}
	UPDATE_COMMAND ""
	SOURCE_SUBDIR "build/cmake"
	INSTALL_COMMAND ""
	CMAKE_CACHE_ARGS
		-DCMAKE_C_COMPILER:PATH=${Compiler_C}
		-DCMAKE_CXX_COMPILER:PATH=${Compiler_CXX}
)

ExternalProject_Get_Property(glew_external_download BINARY_DIR)
ExternalProject_Get_Property(glew_external_download SOURCE_DIR)

SET(GLEW_LIBRARY "${BINARY_DIR}/lib/Release" CACHE INTERNAL "")
set(GLEW_INCLUDE_DIR ${SOURCE_DIR}/include CACHE INTERNAL "")

add_library(glew_external STATIC IMPORTED)

#set(glew_LIBRARIES
#  ${glew_LIBRARY_DIR}/${prefix}glew32${suffix}
#  ${glew_LIBRARY_DIR}/${prefix}libglew32${suffix}
#  CACHE INTERNAL ""
#)

message(STATUS "glew_DIR: ${GLEW_LIBRARY}")
