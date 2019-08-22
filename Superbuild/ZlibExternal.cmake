#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
SET(zlib_GIT_TAG "origin/master")


# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Zlib_external_download
  GIT_REPOSITORY "https://github.com/madler/zlib.git"
  GIT_TAG ${zlib_GIT_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DCMAKE_CXX_FLAGS:STATIC=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STATIC=${CMAKE_C_FLAGS}
    -DDO_ZLIB_MANGLE:BOOL=${DO_ZLIB_MANGLE}
)

ExternalProject_Get_Property(Zlib_external_download BINARY_DIR)
ExternalProject_Get_Property(Zlib_external_download SOURCE_DIR)

if(MSVC)
  SET(Zlib_LIBRARY_DIR "${BINARY_DIR};${BINARY_DIR}/Debug;${BINARY_DIR}/Release" CACHE INTERNAL "")
else()
  SET(Zlib_LIBRARY_DIR ${BINARY_DIR} CACHE INTERNAL "")
endif()

SET(Zlib_INCLUDE_DIR ${SOURCE_DIR} CACHE INTERNAL "")

add_library(Zlib STATIC IMPORTED)

MESSAGE(STATUS "Zlib_LIBRARY_DIR: ${Zlib_LIBRARY_DIR}")
