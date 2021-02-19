#/*
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

cmake_minimum_required(VERSION 3.0)
project(Fluorender)

# Places the final runtime binary in a bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set the standard to C++17 as std::apply and std::filesystem are used
# The compilers are also cached internally so they may be passed in the superbuild
#
# TODO: Throw a fatal error if a C++17 compiler is not found.
set(CMAKE_CXX_STANDARD 17)
set(Compiler_C ${CMAKE_C_COMPILER} CACHE INTERNAL "")
set(Compiler_CXX ${CMAKE_CXX_COMPILER} CACHE INTERNAL "")

option(Debug_Mode "Debug on by default" ON)

# Check if nasm is installed with Linux. Throw a fatal error if it is not found
if(UNIX AND NOT APPLE)
  find_program(NASM_EXE NAMES nasm)
  
  if(NOT NASM_EXE)
    MESSAGE(FATAL_ERROR "
      nasm was not found on your system and is required for FFmpeg, please install with: 
      
      - sudo apt update
      - sudo apt install nasm 
    
      then run Cmake again. 
    ")
  endif()
endif()

# Finds the compiler being used
include(config.cmake)

# This will determine the static library suffix's, may not be needed.
include(setPreSuffix.cmake)

# Required packages
message("Looking for OpenGL")
find_package(OpenGL REQUIRED)
message("Found OpenGL, looking for OpenCL")
find_package(OpenCL REQUIRED)
message("Found, looking for Qt components")
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
message("Found, looking for Java Components")
find_package(Java COMPONENTS Development REQUIRED)
find_package(JNI REQUIRED)
message("Found, configuring...")

#headers
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5OpenGL_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${JNI_INCLUDE_DIRS})
include_directories(Base)
include_directories(InternalDepends)
include_directories(Processor)
include_directories(Renderer)

#defines
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5OpenGL_DEFINITIONS})
add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})
add_definitions(-DNOMINMAX)

# This will look for the Superbuild
Include(${Superbuild})

# These are defined here because Superbuild.cmake defines their locations
set(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIR})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Teem_INCLUDE_DIR})
include_directories(${FFmpeg_INCLUDE_DIR})
include_directories(${glm_INCLUDE_DIR})
include_directories(${pole_INCLUDE_DIR})
include_directories(${TIFF_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${pugi_INCLUDE_DIR})
include_directories(${OpenCL_INCLUDE_DIR})

add_subdirectory(InternalDepends)
add_subdirectory(Base)
#add_subdirectory(Tester)
