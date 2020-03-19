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

# this is libpng's official github repo. The tag is the lastest release. 
# This project depends on Zlib.
SET(libpng_GIT_URL "https://gitlab.com/libtiff/libtiff.git")
SET(libpng_GIT_TAG "v4.0.10")
SET(libpng_DEPENDENCIES "Zlib_external_download")


#needs to be combined here or the cmake file will not find zlib
set(Zlibincludes "${Zlib_LIBRARY_DIR};${Zlib_INCLUDE_DIR}")

# This magic was found on stackoverflow to pass in multiple include directories.
set(Zlib_Root ${Zlib_LIBRARY_DIR})


ExternalProject_Add(libtiff_external_download
  DEPENDS ${libpng_DEPENDENCIES}
  GIT_REPOSITORY ${libpng_GIT_URL}
  GIT_TAG ${libpng_GIT_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  LIST_SEPARATOR |
  CMAKE_ARGS ${libtiff_external_CMAKE_ARGS} -DCMAKE_PREFIX_PATH=${Zlib_Root}
  CMAKE_CACHE_ARGS
    -DCMAKE_C_COMPILER:PATH=${Compiler_C}
    -DCMAKE_CXX_COMPILER:PATH=${Compiler_CXX}
    -DZLIB_INCLUDE_DIR:PATH=${Zlibincludes}
)

ExternalProject_Get_Property(libtiff_external_download BINARY_DIR)
ExternalProject_Get_Property(libtiff_external_download SOURCE_DIR)

SET(libtiff_LIBRARY_DIR ${BINARY_DIR}/libtiff CACHE INTERNAL "")
SET(TIFF_INCLUDE_DIR "${SOURCE_DIR}/libtiff;${BINARY_DIR}/libtiff" CACHE INTERNAL "")
SET(TIFF_LIBRARY ${libtiff_LIBRARY_DIR} CACHE INTERNAL "")

add_library(libtiff_external SHARED IMPORTED)

set(TIFF_LIBRARIES
  ${TIFF_LIBRARY}/${prefix}tiff${suffix} CACHE INTERNAL ""
)

MESSAGE(STATUS "libtiff_DIR: ${libtiff_LIBRARY_DIR}")
