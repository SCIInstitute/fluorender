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

# Raul Ramirez created a CMake file for pole. The original author of pole
# stated that they did not beleive this needed to be created into a library
# and should only be compiled with the project. However for Fluorender, pole
# is needed as a library. So Raul Ramirez created a CMake project for pole.
set(pole_URL "https://github.com/Sailanarmo/pole.git")
set(pole_TAG "origin/master")


ExternalProject_Add(pole_external_download
  GIT_REPOSITORY ${pole_URL}
  GIT_TAG ${pole_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_C_COMPILER:PATH=${Compiler_C}
    -DCMAKE_CXX_COMPILER:PATH=${Compiler_CXX}
)

ExternalProject_Get_Property(pole_external_download BINARY_DIR)
ExternalProject_Get_Property(pole_external_download SOURCE_DIR)

SET(pole_LIBRARY_DIR "${BINARY_DIR}/Debug" CACHE INTERNAL "")
SET(pole_INCLUDE_DIR ${SOURCE_DIR} CACHE INTERNAL "")

add_library(pole_external SHARED IMPORTED)

MESSAGE(STATUS "pole_LIBRARY_DIR: ${pole_LIBRARY_DIR}")
