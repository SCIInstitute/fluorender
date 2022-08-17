# /*
# For more information, please see: http://software.sci.utah.edu
# 
# The MIT License
# 
# Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

# This is an CMake configuration file for FluoRender

cmake_minimum_required(VERSION 3.19.0)

IF((COMMAND cmake_policy) AND NOT (CMAKE_MAJOR_VERSION LESS 3))
  CMAKE_POLICY(SET CMP0040 NEW)
  CMAKE_POLICY(SET CMP0043 NEW)
ENDIF()

SET(CMAKE_CXX_STANDARD 17 CACHE STRING "Default value for CXX_STANDARD property of targets")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FluoRender LANGUAGES C CXX)

# Be sure to update the Versions and Date for each release!!!!
set(FLUORENDER_TITLE "FluoRender")
set(FLUORENDER_VERSION_MAJOR 3)
set(FLUORENDER_VERSION_MINOR 0)
set(FLUORENDER_VERSION_PATCH 0)
set(FLUORENDER_VERSION_COPYRIGHT "December 2022")

set(FLUORENDER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src/FluoRender)
set(FLUORENDER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Src/FluoRender)

# Set up easy to use includes for the common directory
SET(FLUORENDER_COMMON_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/Src
    ${FLUORENDER_SOURCE_DIR}/include
    ${FLUORENDER_BINARY_DIR}/include
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

if(APPLE)
  set(ARCHITECTURE 64)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
elseif(UNIX)
  set(ARCHITECTURE 64)
#  set(RELEASE_FLAGS ${RELEASE_FLAGS} -O3 -march=corei7-avx)
#  set(DEBUG_FLAGS ${DEBUG_FLAGS} -O0 -march=corei7-avx -ggdb)
elseif(MINGW)
  set(ARCHITECTURE 64)
elseif(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0602)
  add_definitions(-D_HAS_STD_BYTE=0)

  if(MSVC)
    SET(MSVC_INCREMENTAL_DEFAULT OFF)
  endif()

  if(${CMAKE_SIZEOF_VOID_P} MATCHES "8")
    set(ARCHITECTURE 64)
  else()
    set(ARCHITECTURE 32)
  endif()
endif()

# architecture specific rules
if(${ARCHITECTURE} MATCHES "64")
  if(APPLE)
    set(ARCH_FLAGS "-m64 -arch x86_64")
  endif()

  add_definitions(-DFLUORENDER_ARCH="64bit")
else()
  if(APPLE)
    set(ARCH_FLAGS "-m32 -arch i386")
  endif()

  add_definitions(-DFLUORENDER_ARCH="32bit")
endif()

# static compile
add_definitions(-DSTATIC_COMPILE)

if(APPLE OR UNIX OR MINGW)
  set(CFLAGS "-fPIC")
  set(CXXFLAGS "-fPIC ")
elseif(WIN32)
  if(MSVC)
    #add_definitions(-DSEQAN_C++11_STANDARD=ON)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-Dinline=__inline)
    add_definitions("/wd4273")
    add_definitions(-D_XKEYCHECK_H)
    add_definitions(-DNOMINMAX)
    set(CFLAGS "")
    set(CXXFLAGS "/EHsc")
    # make sure the reference option is turned off and not incremental build linking
    STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
    STRING(REPLACE "INCREMENTAL:NO:NO" "INCREMENTAL:NO" replacementFlags1 ${replacementFlags})
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO /OPT:NOREF ${replacementFlags1}")
    STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags2 ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
    STRING(REPLACE "INCREMENTAL:NO:NO" "INCREMENTAL:NO" replacementFlags3 ${replacementFlags2})
    SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO /OPT:NOREF ${replacementFlags3}")

    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /OPT:NOREF")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -MDd")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -MDd")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MD")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -MD")
  endif()

  # To build for Win7, disable xinput
  SET(USE_XINPUT ON CACHE BOOL "Use Xinput on Windows")
  if (USE_XINPUT)
    add_definitions(-DUSE_XINPUT=1)
  endif()
endif()

set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}")
set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")

# Get the required and optional packages. 

# OpenGL - find first 
find_package(OpenGL REQUIRED)

# Platform specific rules for OpenGL
if(APPLE)
  if(NOT OPEN_GL_HEADER_LOC)
    add_definitions(-DOPEN_GL_HEADER_LOC=<OpenGL/gl.h>)
  endif()
  if(NOT OPEN_GLU_HEADER_LOC)
    add_definitions(-DOPEN_GLU_HEADER_LOC=<OpenGL/glu.h>)
  endif()
elseif(UNIX OR MINGW)
  # Linux
  #if(NOT OPEN_GL_HEADER_LOC)
  #  add_definitions(-DOPEN_GL_HEADER_LOC=<OpenGL/gl.h>)
  #endif()
  #if(NOT OPEN_GLU_HEADER_LOC)
  #  add_definitions(-DOPEN_GLU_HEADER_LOC=<OpenGL/glu.h>)
  #endif()
elseif(WIN32)
  # Windows
  if(NOT OPEN_GL_HEADER_LOC)
    add_definitions(-DOPEN_GL_HEADER_LOC=<GL/gl.h>)
  endif()
  if(NOT OPEN_GLU_HEADER_LOC)
    add_definitions(-DOPEN_GLU_HEADER_LOC=<GL/glu.h>)
  endif()
endif()

# Qt - find after OpenGL
# Qt needs the XCB package
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${QT_DIR}/lib/cmake/Qt6/3rdparty/extra-cmake-modules/find-modules")
find_package(XCB COMPONENTS XCB)

# Find all of the Qt6 componenets.
set(Qt6_DIR             "${QT_DIR}/lib/cmake/Qt6")
set(Qt6Core_DIR         "${QT_DIR}/lib/cmake/Qt6Core")
set(Qt6CoreTools_DIR    "${QT_DIR}/lib/cmake/Qt6CoreTools")
set(Qt6GuiTools_DIR     "${QT_DIR}/lib/cmake/Qt6GuiTools")
set(Qt6WidgetsTools_DIR "${QT_DIR}/lib/cmake/Qt6WidgetsTools")
set(Qt6DBusTools_DIR    "${QT_DIR}/lib/cmake/Qt6DBusTools")

set(QT_COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets UiPlugin)
string(REGEX REPLACE "([^;]+)(;|$)" "Qt::\\1\\2" QT_LIBRARIES "${QT_COMPONENTS}")

find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})

# Qt specific bits for building.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# The Qt Advanced Docking package is optional just in case a minimal
# build is desired.

# Use the FluoRender specific Findqtadvanceddocking.cmake as the
# default qtadvanceddockingConfig.cmake does not set everything needed.
#set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${qtadvanceddocking_DIR}/lib/cmake/qtadvanceddocking)

if(DEFINED qtadvanceddocking_DIR)
  find_package(qtadvanceddocking REQUIRED COMPONENTS qtadvanceddocking)
else(DEFINED qtadvanceddocking_DIR)
  find_package(qtadvanceddocking OPTIONAL COMPONENTS qtadvanceddocking)
endif(DEFINED qtadvanceddocking_DIR)

# Boost - Only find static libraries.
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system chrono filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# FFMEG
if(UNIX OR APPLE OR MINGW)
  find_package(FFMPEG REQUIRED)
  include_directories(${FFMPEG_INCLUDE_DIR})
endif()

# FreeType
if(UNIX OR APPLE OR MINGW)
  find_package(Freetype REQUIRED)
  include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

# JNI includes
find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})

# OpenCL
if(UNIX OR APPLE OR MINGW)
  find_package(OpenCL REQUIRED)
  include_directories(${OpenCL_INCLUDE_DIRS})
endif()

# Tiff
find_package(TIFF REQUIRED)
include_directories(${TIFF_INCLUDE_DIRS})

# Zlib
find_package(ZLIB)
include_directories(${ZLIB_INCLUDE_DIRS})

# OS X Frameworks
if(APPLE)
  find_library(BZ2_LIBRARIES bz2 REQUIRED)
  find_library(ICONV_LIBRARIES Iconv REQUIRED)

  find_library(CORE_FOUNDATION_LIBRARIES CoreFoundation REQUIRED)
  find_library(CORE_MEDIA_LIBRARIES CoreMedia REQUIRED)
  find_library(CORE_VIDEO_LIBRARIES CoreVideo REQUIRED)
  find_library(AUDIO_TOOLBOX_LIBRARIES AudioToolbox REQUIRED)
  find_library(VIDEO_TOOLBOX_LIBRARIES VideoToolbox REQUIRED)
  find_library(VIDEO_DECODE_ACCELERATION_LIBRARIES VideoDecodeAcceleration REQUIRED)
  find_library(SECURITY_LIBRARIES Security REQUIRED)

  MARK_AS_ADVANCED(BZ2_LIBRARIES
                   ICONV_LIBRARIES
                   CORE_FOUNDATION_LIBRARIES
                   CORE_MEDIA_LIBRARIES
                   CORE_VIDEO_LIBRARIES
                   AUDIO_TOOLBOX_LIBRARIES
                   VIDEO_TOOLBOX_LIBRARIES
                   VIDEO_DECODE_ACCELERATION_LIBRARIES
                   SECURITY_LIBRARIES)
endif()

# Sets up the FluoRender HAVE_* specific defines - must be done after
# all packages are found.
CONFIGURE_FILE(${FLUORENDER_SOURCE_DIR}/include/fluorender-cmake.h.in
               ${FLUORENDER_BINARY_DIR}/include/fluorender-config.h @ONLY IMMEDIATE)

# Thirdparty libraries - each should really be built as individual
# shared libraries and linked accrodingly.

# Eigen
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/Eigen)

# Glew
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/glew)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/glew/GL)
file(GLOB glew_srcs    ${CMAKE_CURRENT_SOURCE_DIR}/Src/glew/*.c)
file(GLOB glew_hdrs    ${CMAKE_CURRENT_SOURCE_DIR}/Src/glew/*.h)
file(GLOB glew_gl_hdrs ${CMAKE_CURRENT_SOURCE_DIR}/Src/glew/GL/*.h)
add_library(GLEW_OBJ OBJECT ${glew_srcs} ${glew_hdrs} ${glew_gl_hdrs})

# Glm - headers only
add_definitions(-DGLM_FORCE_RADIANS=1)

# FFMpeg
if(WIN32)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/ffmpeg/include)
  file(GLOB FFMPEG_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/ffmpeg/lib/Win64/*.a ${CMAKE_CURRENT_SOURCE_DIR}/Src/ffmpeg/lib/Win64/*.lib)
  add_definitions(-D__STDC_CONSTANT_MACROS)
elseif(APPLE)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/ffmpeg/include)
  file(GLOB FFMPEG_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/ffmpeg/lib/OSX/*.a)
endif()

# Freetype
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/freetype/include)
if(WIN32)
  file(GLOB FREETYPE_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/freetype/lib/*.lib)
endif()

# MaskInt
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/MaskInt)
file(GLOB maskint_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/MaskInt/*.h)
file(GLOB maskint_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/MaskInt/*.cpp)

# Nikon
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/include)
if(WIN32)
  if(${ARCHITECTURE} MATCHES 64)
    file(GLOB ND2_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/lib/*.lib)
  endif()
elseif(APPLE)
   file(GLOB ND2_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/lib/*.dylib)
endif()

# OpenCL
if(WIN32)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/OpenCL/include)
  if(${ARCHITECTURE} MATCHES 64)
    file(GLOB OpenCL_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/OpenCL/lib/x86_64/*.lib)
  elseif(${ARCHITECTURE} MATCHES 32)
    file(GLOB OpenCL_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/OpenCL/lib/x86/*.lib)
  endif()
endif()

# OpenVR
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/OpenVR/headers)
if(WIN32)
  if(${ARCHITECTURE} MATCHES 64)
    file(GLOB OpenVR_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/Src/OpenVR/lib/win64/*.lib)
  endif()
endif()

# Pole
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/pole)
file(GLOB pole_srcs ${CMAKE_CURRENT_SOURCE_DIR}/Src/pole/pole.cpp)
file(GLOB pole_hdrs ${CMAKE_CURRENT_SOURCE_DIR}/Src/pole/pole.h)
add_library(POLE_OBJ OBJECT ${pole_srcs} ${pole_hdrs})

# ReadGMM
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/ReadGMM)
file(GLOB readgmm_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/ReadGMM/*.h)
file(GLOB readgmm_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/ReadGMM/*.cpp)

# SynthBB
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/SynthBB)
file(GLOB synthbb_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/SynthBB/*.h)
file(GLOB synthbb_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/SynthBB/*.cpp)

# Teem
add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

if(${ARCHITECTURE} MATCHES "64")
  add_definitions(-DTEEM_32BIT=0)
else()
  add_definitions(-DTEEM_32BIT=1)
endif()

if(ZLIB_FOUND)
  add_definitions(-DTEEM_ZLIB=1)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/teem)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Biff)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Air)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Hest)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Nrrd)
file(GLOB air_src  ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Air/*.c)
file(GLOB air_hdr  ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Air/*.h)
file(GLOB hest_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Hest/*.c)
file(GLOB hest_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Hest/*.h)
file(GLOB biff_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Biff/*.c)
file(GLOB biff_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Biff/*.h)
file(GLOB nrrd_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Nrrd/*.c)
file(GLOB nrrd_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/teem/Nrrd/*.h)
add_library(TEEM_OBJ OBJECT ${air_src} ${hest_src} ${nrrd_src} ${biff_src}
                            ${air_hdr} ${hest_hdr} ${nrrd_hdr} ${biff_hdr})

# XInput
if(WIN32)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/Wacom/Include)
  file(GLOB wac_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/Wacom/*.cpp)
  file(GLOB wac_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/Wacom/*.h)
  
  add_library(WACUTILS_OBJ OBJECT
    ${wac_src} ${wac_hdr})

  if(USE_XINPUT)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/XInput)
    file(GLOB xip_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/XInput/*.cpp)
    file(GLOB xip_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/XInput/*.h)
    add_library(XINPUT_OBJ OBJECT
      ${xip_src} ${xip_hdr})
  endif()
endif()

# Tester
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Src/Tester)
file(GLOB tester_hdr ${CMAKE_CURRENT_SOURCE_DIR}/Src/Tester/*.h)
file(GLOB tester_src ${CMAKE_CURRENT_SOURCE_DIR}/Src/Tester/*.cpp)

# FluoRender - Each directory should really be built as individual
# shared libraries and linked accrodingly.

# Utilities
include_directories(${FLUORENDER_SOURCE_DIR}/Utilities)
file(GLOB utilities_src ${FLUORENDER_SOURCE_DIR}/Utilities/*.cpp)
file(GLOB utilities_hdr ${FLUORENDER_SOURCE_DIR}/Utilities/*.h)
add_library(UTILITIES_OBJ OBJECT
  ${utilities_src} ${utilities_hdr})

# Agents
include_directories(${FLUORENDER_SOURCE_DIR}/Agents)
file(GLOB agt_src ${FLUORENDER_SOURCE_DIR}/Agents/*.cpp)
file(GLOB agt_hdr ${FLUORENDER_SOURCE_DIR}/Agents/*.hpp)
add_library(AGENTS_OBJ OBJECT
  ${agt_src} ${agt_hdr})

# Animators
include_directories(${FLUORENDER_SOURCE_DIR}/Animator)
file(GLOB ani_src ${FLUORENDER_SOURCE_DIR}/Animator/*.cpp)
file(GLOB ani_hdr ${FLUORENDER_SOURCE_DIR}/Animator/*.h)
add_library(ANIMATORS_OBJ OBJECT
  ${ani_src} ${ani_hdr})

# Calculate
include_directories(${FLUORENDER_SOURCE_DIR}/Calculate)
file(GLOB calc_src ${FLUORENDER_SOURCE_DIR}/Calculate/*.cpp)
file(GLOB calc_hdr ${FLUORENDER_SOURCE_DIR}/Calculate/*.h)
add_library(CALCULATE_OBJ OBJECT
  ${calc_src} ${calc_hdr})

# Cluster
include_directories(${FLUORENDER_SOURCE_DIR}/Cluster)
file(GLOB clstr_src ${FLUORENDER_SOURCE_DIR}/Cluster/*.cpp)
file(GLOB clstr_hdr ${FLUORENDER_SOURCE_DIR}/Cluster/*.h)
add_library(CLUSTER_OBJ OBJECT
  ${clstr_src} ${clstr_hdr})

# Components
include_directories(${FLUORENDER_SOURCE_DIR}/Components)
file(GLOB cmp_src ${FLUORENDER_SOURCE_DIR}/Components/*.cpp)
file(GLOB cmp_hdr ${FLUORENDER_SOURCE_DIR}/Components/*.h)
add_library(COMPONENTS_OBJ OBJECT
  ${cmp_src} ${cmp_hdr})

# Converters
include_directories(${FLUORENDER_SOURCE_DIR}/Converters)
file(GLOB cvt_src ${FLUORENDER_SOURCE_DIR}/Converters/*.cpp)
file(GLOB cvt_hdr ${FLUORENDER_SOURCE_DIR}/Converters/*.h)
add_library(CONVERTERS_OBJ OBJECT
  ${cvt_src} ${cvt_hdr})

# Debug
include_directories(${FLUORENDER_SOURCE_DIR}/Debug)
file(GLOB dbg_src ${FLUORENDER_SOURCE_DIR}/Debug/*.cpp)
file(GLOB dbg_hdr ${FLUORENDER_SOURCE_DIR}/Debug/*.hpp)
add_library(DEBUG_OBJ OBJECT
  ${dbg_src} ${dbg_hdr})

# Distance
include_directories(${FLUORENDER_SOURCE_DIR}/Distance)
file(GLOB dist_src ${FLUORENDER_SOURCE_DIR}/Distance/*.cpp)
file(GLOB dist_hdr ${FLUORENDER_SOURCE_DIR}/Distance/*.h)
add_library(DISTANCE_OBJ OBJECT
  ${dist_src} ${dist_hdr})

# FLIVR
include_directories(${FLUORENDER_SOURCE_DIR}/FLIVR)
file(GLOB flivr_src ${FLUORENDER_SOURCE_DIR}/FLIVR/*.cpp)
file(GLOB flivr_hdr ${FLUORENDER_SOURCE_DIR}/FLIVR/*.h)
add_library(FLIVR_OBJ OBJECT
  ${flivr_src} ${flivr_hdr})

# Flobject
include_directories(${FLUORENDER_SOURCE_DIR}/Flobject)
file(GLOB obj_src ${FLUORENDER_SOURCE_DIR}/Flobject/*.cpp)
file(GLOB obj_hdr ${FLUORENDER_SOURCE_DIR}/Flobject/*.hpp)
add_library(FLOBJECT_OBJ OBJECT
  ${obj_src} ${obj_hdr})

# Formats
include_directories(${FLUORENDER_SOURCE_DIR}/Formats)
file(GLOB fmt_src ${FLUORENDER_SOURCE_DIR}/Formats/*.cpp)
file(GLOB fmt_hdr ${FLUORENDER_SOURCE_DIR}/Formats/*.h)
file(GLOB fmt_rsc ${FLUORENDER_SOURCE_DIR}/Formats/*.class)
add_library(FORMATS_OBJ OBJECT
  ${fmt_src} ${fmt_hdr} ${fmt_rsc})

# Global
include_directories(${FLUORENDER_SOURCE_DIR}/Global)
file(GLOB glb_src ${FLUORENDER_SOURCE_DIR}/Global/*.cpp)
file(GLOB glb_hdr ${FLUORENDER_SOURCE_DIR}/Global/*.hpp)
add_library(GLOBAL_OBJ OBJECT
  ${glb_src} ${glb_hdr})

# Input
include_directories(${FLUORENDER_SOURCE_DIR}/Input)
file(GLOB input_src ${FLUORENDER_SOURCE_DIR}/Input/*.cpp)
file(GLOB input_hdr ${FLUORENDER_SOURCE_DIR}/Input/*.hpp)
add_library(INPUT_OBJ OBJECT
  ${input_src} ${input_hdr})

# Scenegraph
include_directories(${FLUORENDER_SOURCE_DIR}/Scenegraph)
file(GLOB scg_src ${FLUORENDER_SOURCE_DIR}/Scenegraph/*.cpp)
file(GLOB scg_hdr ${FLUORENDER_SOURCE_DIR}/Scenegraph/*.hpp)
add_library(SCENEGRAPH_OBJ OBJECT
  ${scg_src} ${scg_hdr})

# Script
include_directories(${FLUORENDER_SOURCE_DIR}/Script)
file(GLOB scr_src ${FLUORENDER_SOURCE_DIR}/Script/*.cpp)
file(GLOB scr_hdr ${FLUORENDER_SOURCE_DIR}/Script/*.h)
add_library(SCRIPT_OBJ OBJECT
  ${scr_src} ${scr_hdr})

# Selection
include_directories(${FLUORENDER_SOURCE_DIR}/Selection)
file(GLOB sel_src ${FLUORENDER_SOURCE_DIR}/Selection/*.cpp)
file(GLOB sel_hdr ${FLUORENDER_SOURCE_DIR}/Selection/*.h)
add_library(SELECTION_OBJ OBJECT
  ${sel_src} ${sel_hdr})

# Timer
include_directories(${FLUORENDER_SOURCE_DIR}/Timer)
file(GLOB timer_src ${FLUORENDER_SOURCE_DIR}/Timer/*.cpp)
file(GLOB timer_hdr ${FLUORENDER_SOURCE_DIR}/Timer/*.hpp)
add_library(TIMER_OBJ OBJECT
  ${timer_src} ${timer_hdr})

# Tracking
include_directories(${FLUORENDER_SOURCE_DIR}/Tracking)
file(GLOB trk_src ${FLUORENDER_SOURCE_DIR}/Tracking/*.cpp)
file(GLOB trk_hdr ${FLUORENDER_SOURCE_DIR}/Tracking/*.h)
add_library(TRACKING_OBJ OBJECT
  ${trk_src} ${trk_hdr})

# Types
include_directories(${FLUORENDER_SOURCE_DIR}/Types)
file(GLOB typ_src ${FLUORENDER_SOURCE_DIR}/Types/*.cpp)
file(GLOB typ_hdr ${FLUORENDER_SOURCE_DIR}/Types/*.h)
add_library(TYPES_OBJ OBJECT
  ${typ_src} ${typ_hdr})

# Video
include_directories(${FLUORENDER_SOURCE_DIR}/Video)
file(GLOB vid_src ${FLUORENDER_SOURCE_DIR}/Video/*.cpp)
file(GLOB vid_hdr ${FLUORENDER_SOURCE_DIR}/Video/*.h)
add_library(VIDEO_OBJ OBJECT
  ${vid_src} ${vid_hdr})

# Build the UI as a separate shared library
add_subdirectory(${FLUORENDER_SOURCE_DIR}/UI)

# For Apple and Windoz set the icns file containing icons
if(APPLE)
  # Icon property in the Info.plist
  set(MACOSX_BUNDLE_ICON_FILE FluoRender_icon.icns)
  # Where to find and install the icon
  set(FLUORENDER_ICON "${FLUORENDER_SOURCE_DIR}/UI/icons/FluoRender_icon.icns")
  set_SOURCE_FILES_PROPERTIES(${FLUORENDER_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
elseif(WIN32)
  set(FLUORENDER_ICON "${FLUORENDER_SOURCE_DIR}/UI/icons/fluorenderIcon.ico")
  set(FLUORENDER_ICON_RC "${FLUORENDER_SOURCE_DIR}/UI/icons/FluoRender.rc")
  add_definitions(-DICON_RC_FILE="${FLUORENDER_ICON}")
  set(main_src ${main_src} ${FLUORENDER_ICON_RC})
endif()

qt_add_executable(FluoRender MACOSX_BUNDLE WIN32
#  ${main_src}
  ${FLUORENDER_SOURCE_DIR}/Main/main.cpp
  FluoRender.qrc
  ${FLUORENDER_ICON}
  $<TARGET_OBJECTS:AGENTS_OBJ>
  $<TARGET_OBJECTS:ANIMATORS_OBJ>
  $<TARGET_OBJECTS:CALCULATE_OBJ>
  $<TARGET_OBJECTS:CLUSTER_OBJ>
  $<TARGET_OBJECTS:COMPONENTS_OBJ>
  $<TARGET_OBJECTS:CONVERTERS_OBJ>
  $<TARGET_OBJECTS:DEBUG_OBJ>
  $<TARGET_OBJECTS:DISTANCE_OBJ>
  $<TARGET_OBJECTS:FLIVR_OBJ>
  $<TARGET_OBJECTS:FLOBJECT_OBJ>
  $<TARGET_OBJECTS:FORMATS_OBJ>
  $<TARGET_OBJECTS:GLOBAL_OBJ>
  $<TARGET_OBJECTS:INPUT_OBJ>
  $<TARGET_OBJECTS:SCENEGRAPH_OBJ>
  $<TARGET_OBJECTS:SCRIPT_OBJ>
  $<TARGET_OBJECTS:SELECTION_OBJ>
  $<TARGET_OBJECTS:TIMER_OBJ>
  $<TARGET_OBJECTS:TRACKING_OBJ>
  $<TARGET_OBJECTS:TYPES_OBJ>
  $<TARGET_OBJECTS:UTILITIES_OBJ>
  $<TARGET_OBJECTS:VIDEO_OBJ>
  $<TARGET_OBJECTS:GLEW_OBJ>
  $<TARGET_OBJECTS:POLE_OBJ>
  $<TARGET_OBJECTS:TEEM_OBJ>)

add_executable(Tester MACOSX_BUNDLE WIN32
  ${tester_src}
  $<TARGET_OBJECTS:TYPES_OBJ>
  $<TARGET_OBJECTS:FLOBJECT_OBJ>)

# FluoRender specific.
target_compile_options(FluoRender PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(FluoRender PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")

include_directories(${FLUORENDER_COMMON_INCLUDES})

# For some reason this call is not working.
#target_include_directories(FluoRender PUBLIC
#			   ${FLUORENDER_COMMON_INCLUDES})

# Platform specific sources
if(WIN32)
  target_sources(FluoRender PRIVATE
    $<TARGET_OBJECTS:WACUTILS_OBJ>)

  if (USE_XINPUT)
    target_sources(FluoRender PRIVATE
      $<TARGET_OBJECTS:XINPUT_OBJ>)
  endif()
endif()

# Link libraries
target_link_libraries(FluoRender PUBLIC
  FluoRenderUI
  ${FFMPEG_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCL_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${TIFF_LIBRARIES}
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${ND2_LIBRARIES})

# OS X frameworks
if(APPLE)
  target_link_libraries(FluoRender PUBLIC
    ${BZ2_LIBRARIES}
    ${ICONV_LIBRARIES}
    ${CORE_FOUNDATION_LIBRARIES}
    ${CORE_MEDIA_LIBRARIES}
    ${CORE_VIDEO_LIBRARIES}
    ${AUDIO_TOOLBOX_LIBRARIES}
    ${VIDEO_TOOLBOX_LIBRARIES}
    ${VIDEO_DECODE_ACCELERATION_LIBRARIES}
    ${SECURITY_LIBRARIES}
#    ${JNI_LIBRARIES}
    )
elseif(UNIX OR MINGW)
  target_link_libraries(FluoRender PUBLIC
    pthread
    dl
#    ${JNI_LIBRARIES}
    )
#  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -ldl")
elseif(WIN32)
  target_link_libraries(FluoRender
    secur32.lib
    XInput.lib)
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy Java code dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Java_Code/ImageJ_Reader/target/classes"
  "$<TARGET_FILE_DIR:FluoRender>/Java_Code")
  
# Copy openCL examples to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/CL_code"
  "$<TARGET_FILE_DIR:FluoRender>/CL_code")

# Copy font dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Fonts"
  "$<TARGET_FILE_DIR:FluoRender>/Fonts")

# Copy script dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Scripts"
  "$<TARGET_FILE_DIR:FluoRender>/Scripts")

# Copy template dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Templates"
  "$<TARGET_FILE_DIR:FluoRender>/Templates")

# Copy command dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Commands"
  "$<TARGET_FILE_DIR:FluoRender>/Commands")

# Settings files
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Settings"
  "$<TARGET_FILE_DIR:FluoRender>")

if(APPLE)
  add_custom_command(TARGET FluoRender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings_Mac/fluorender.set"
    "$<TARGET_FILE_DIR:FluoRender>")
endif()

# Copy data dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Data"
  "$<TARGET_FILE_DIR:FluoRender>/Data")

# Benchmark files
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/Benchmark"
  "$<TARGET_FILE_DIR:FluoRender>")

# Openvr
if(WIN32)
  add_custom_command(TARGET FluoRender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/OpenVR/lib/win64/openvr_api.dll"
    "$<TARGET_FILE_DIR:FluoRender>")
endif()

# Nd2
if(APPLE)
  add_custom_command(TARGET FluoRender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/lib/libNd2File.dylib"
    "$<TARGET_FILE_DIR:FluoRender>"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/lib/libNd2ReadSdk.dylib"
    "$<TARGET_FILE_DIR:FluoRender>")
  add_custom_command(TARGET FluoRender POST_BUILD COMMAND 
    ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/"
    $<TARGET_FILE:FluoRender>)
elseif(WIN32)
  add_custom_command(TARGET FluoRender POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/bin/limfile.dll"
    "$<TARGET_FILE_DIR:FluoRender>"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/bin/nd2readsdk.dll"
    "$<TARGET_FILE_DIR:FluoRender>"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/bin/tiff.dll"
    "$<TARGET_FILE_DIR:FluoRender>"
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/nd2/bin/zlib1.dll"
    "$<TARGET_FILE_DIR:FluoRender>"
    )
endif()

# Make the executable directory
if(WIN32)
  add_custom_target(make_exe_dir ALL 
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:FluoRender>/Executables")
else()
  add_custom_command(TARGET FluoRender POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:FluoRender>/Executables")
endif()
