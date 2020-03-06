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

# Raul Ramirez created a CMake file for pugi. The original author of pugi
# stated that they did not beleive this needed to be created into a library
# and should only be compiled with the project. However for Fluorender, pugi
# is needed as a library. So Raul Ramirez created a CMake project for pugi.
set(pugi_URL "https://github.com/zeux/pugixml")
set(pugi_TAG "v1.10")


ExternalProject_Add(pugi_external_download
  GIT_REPOSITORY ${pugi_URL}
  GIT_TAG ${pugi_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_C_COMPILER:PATH=${Compiler_C}
    -DCMAKE_CXX_COMPILER:PATH=${Compiler_CXX}
)

ExternalProject_Get_Property(pugi_external_download BINARY_DIR)
ExternalProject_Get_Property(pugi_external_download SOURCE_DIR)

if(MSVC AND (NOT ${GeneratorName} STREQUAL "Ninja"))
  SET(pugi_LIBRARY_DIR "${BINARY_DIR};${BINARY_DIR}/Debug;${BINARY_DIR}/Release" CACHE INTERNAL "")
else()
  SET(pugi_LIBRARY_DIR ${BINARY_DIR} CACHE INTERNAL "")
endif()

SET(pugi_INCLUDE_DIR ${SOURCE_DIR}/src CACHE INTERNAL "")

add_library(pugi_external STATIC IMPORTED)

#this is a bandaid fix. Need to really fix this. 
set(pugi_LIBRARIES
  ${BINARY_DIR}/Debug/${prefix}pugixml${suffix} CACHE INTERNAL ""
)

MESSAGE(STATUS "pugi_LIBRARY_DIR: ${pugi_LIBRARY_DIR}")
