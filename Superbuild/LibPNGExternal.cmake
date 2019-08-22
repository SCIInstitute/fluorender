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
SET(libpng_GIT_TAG "origin/master")
SET(libpng_DEPENDENCIES "Zlib")


#needs to be combined here or the cmake file will not find zlib
set(Zlibincludes "${Zlib_LIBRARY_DIR};${Zlib_INCLUDE_DIR}")


if(MSVC)
  string(REPLACE ";" "|" Zlib_Root "${Zlib_LIBRARY_DIR}")
else()
  set(Zlib_Root ${Zlib_LIBRARY_DIR})
endif()

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(LibPNG_external_download
  DEPENDS ${libpng_DEPENDENCIES}
  GIT_REPOSITORY "https://github.com/glennrp/libpng.git"
  GIT_TAG ${libpng_GIT_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  LIST_SEPARATOR |
  CMAKE_ARGS ${LibPNG_external_CMAKE_ARGS} -DCMAKE_PREFIX_PATH=${Zlib_Root}
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DCMAKE_CXX_FLAGS:STATIC=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STATIC=${CMAKE_C_FLAGS}
    -DPNG_STATIC:BOOL=ON
    -DDO_ZLIB_MANGLE:BOOL=${DO_ZLIB_MANGLE}
    -DZLIB_INCLUDE_DIR:PATH=${Zlibincludes}
)

ExternalProject_Get_Property(LibPNG_external_download BINARY_DIR)
ExternalProject_Get_Property(LibPNG_external_download SOURCE_DIR)

if(MSVC)
  SET(LibPNG_LIBRARY_DIR "${BINARY_DIR};${BINARY_DIR}/Debug;${BINARY_DIR}/Release" CACHE INTERNAL "")
else()
  SET(LibPNG_LIBRARY_DIR ${BINARY_DIR} CACHE INTERNAL "")
endif()

SET(LibPNG_INCLUDE_DIR ${SOURCE_DIR} CACHE INTERNAL "")

add_library(LibPNG_external STATIC IMPORTED)

MESSAGE(STATUS "LibPNG_DIR: ${LibPNG_LIBRARY_DIR}")
