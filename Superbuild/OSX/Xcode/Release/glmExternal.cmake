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

# This is the offical repo of glm. The tag is their latest release.
set(glm_URL "https://github.com/g-truc/glm.git")
set(glm_TAG "0.9.9.5")

# We need to check the compiler flag. This will be removed in the future
# since C++17 is required anyways.
include(CheckCXXCompilerFlag)

# This simply performs a check on the compiler. If C++17 is found, GLm is
# then built with C++17. If it isn't, it will look for C++11 and build it 
# with that. Again, this will be removed in the future since C++17 is 
# required anyways.
check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
check_cxx_compiler_flag("/std:c++17" HAVE_FLAG_STD_CXX17VS)
if(HAVE_FLAG_STD_CXX17 OR HAVE_FLAG_STD_CXX17VS)
  message(STATUS "Enabling C++17 Support for GLM")
  set(flag "17")
else()
  check_cxx_compiler_flag(-std=c++11 HAVE_FLAG_STD_CXX11)
  check_cxx_compiler_flag("/std:c++11" HAVE_FLAG_STD_CXX11VS)
  if(HAVE_FLAG_STD_CXX11 OR HAVE_FLAG_STD_CXX11VS)
    message(STATUS "Enabling C++11 Support for GLM")
    set(flag "11")
  else()
    message(FATAL_ERROR "Error, must have at least C++11 in order to continue!")
  endif()
endif()

ExternalProject_Add(glm_external_download
  GIT_REPOSITORY ${glm_URL}
  GIT_TAG ${glm_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS 
    -DCMAKE_C_COMPILER:PATH=${Compiler_C}
    -DCMAKE_CXX_COMPILER:PATH=${Compiler_CXX}
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DGLM_TEST_ENABLE_CXX_${flag}:BOOL=ON
)

ExternalProject_Get_Property(glm_external_download BINARY_DIR)
ExternalProject_Get_Property(glm_external_download SOURCE_DIR)

SET(glm_LIBRARY_DIR ${BINARY_DIR}/glm/Release CACHE INTERNAL "")
SET(glm_INCLUDE_DIR ${SOURCE_DIR} CACHE INTERNAL "")

add_library(glm_external SHARED IMPORTED)

MESSAGE(STATUS "glm_LIBRARY_DIR: ${glm_LIBRARY_DIR}")
