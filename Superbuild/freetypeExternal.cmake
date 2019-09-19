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

# This is freetype's official github. The tag is the lastest release. This
# also depends on zlib and libpng. 
#
# Master Depends simply combines zlib's and libpng's include directories so
# the magical string replace can work when passing into the external project.
set(freetype_URL "https://git.savannah.gnu.org/git/freetype/freetype2.git")
SET(freetype_GIT_TAG "VER-2-10-1")
SET(freetype_DEPENDENCIES "Zlib_external_download;LibPNG_external_download")

set(zlibincludedir "${Zlib_LIBRARY_DIR};${Zlib_INCLUDE_DIR}")
set(libpnginclude "${LibPNG_LIBRARY_DIR};${LibPNG_INCLUDE_DIR}")
set(Master_Depends ${Zlib_LIBRARY_DIR} ${LibPNG_LIBRARY_DIR})

string(REPLACE ";" "|" Master_Root "${Master_Depends}")


ExternalProject_Add(freetype_external_download
  DEPENDS ${freetype_DEPENDENCIES}
  GIT_REPOSITORY ${freetype_URL}
  GIT_TAG ${freetype_GIT_TAG}
  PATCH_COMMAND ""
  INSTALL_DIR ""
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  LIST_SEPARATOR |
  CMAKE_ARGS ${freetype_external_download_CMAKE_ARGS} 
    -DCMAKE_PREFIX_PATH=${Master_Root}
  CMAKE_CACHE_ARGS
    -DCMAKE_C_COMPILER:PATH=${Compiler_C}
    -DCMAKE_CXX_COMPILER:PATH=${Compiler_CXX}
    -DZLIB_INCLUDE_DIR:PATH=${Zlibincludes}
    -DPNG_INCLUDE_DIR:PATH=${libpnginclude}
	  -DPNG_PNG_INCLUDE_DIR:PATH=${libpnginclude}
)

ExternalProject_Get_Property(freetype_external_download BINARY_DIR)
ExternalProject_Get_Property(freetype_external_download SOURCE_DIR)

if(MSVC)
  SET(freetype_LIBRARY_DIR "${BINARY_DIR};${BINARY_DIR}/Debug;${BINARY_DIR}/Release" CACHE INTERNAL "")
else()
  SET(freetype_LIBRARY_DIR ${BINARY_DIR} CACHE INTERNAL "")
endif()

set(freetype_INCLUDE_DIR "${SOURCE_DIR}/include" CACHE INTERNAL "")

add_library(freetype_external STATIC IMPORTED)

MESSAGE(STATUS "freetype_DIR: ${freetype_LIBRARY_DIR}")
