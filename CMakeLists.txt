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

# This is an CMake configuration file for FluoRender

#??
if (NOT WIN32 AND NOT APPLE)
  SET(ENABLE_GDB OFF CACHE BOOL "Enable GDB")
  if (ENABLE_GDB)
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_C_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
  endif()
endif()

# to build for Win7, disable xinput
if (WIN32)
  SET(USE_XINPUT ON CACHE BOOL "Use Xinput on Windows")
  if (USE_XINPUT)
    add_definitions(-DUSE_XINPUT=1)
  endif()
endif()

cmake_minimum_required(VERSION 3.5)

# for MSVC builds
if(WIN32)
  if(MSVC)
    SET(MSVC_INCREMENTAL_DEFAULT OFF)
  endif()
endif()

#if (WIN32 OR APPLE)
#else()
#  set(CMAKE_C_COMPILER "gcc-12")
#  set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
#endif()

project(FluoRender)

IF((COMMAND cmake_policy) AND NOT (CMAKE_MAJOR_VERSION LESS 3))
  CMAKE_POLICY(SET CMP0040 NEW)
  CMAKE_POLICY(SET CMP0043 NEW)
ENDIF()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

if(UNIX OR APPLE OR MINGW)
  set(ARCHITECTURE 64)
elseif(WIN32)
  if(${CMAKE_SIZEOF_VOID_P} MATCHES "8")
    set(ARCHITECTURE 64)
  else()
    set(ARCHITECTURE 32)
  endif()
endif()

if(${ARCHITECTURE} MATCHES "64")
  add_definitions(-DFLUORENDER_ARCH="64bit")
else()
  add_definitions(-DFLUORENDER_ARCH="32bit")
endif()

#if(UNIX)
#  set(RELEASE_FLAGS ${RELEASE_FLAGS} -O3 -march=corei7-avx)
#  set(DEBUG_FLAGS ${DEBUG_FLAGS} -O0 -march=corei7-avx -ggdb)
#endif()

add_definitions(-DFLUORENDER_TITLE="FluoRender")

add_definitions(-DVERSION_MAJOR=2)						# Be sure to update the Versions and Date for each release!!!!
add_definitions(-DVERSION_MINOR=30)						# 
add_definitions(-DVERSION_MAJOR_TAG="2")				# 
add_definitions(-DVERSION_MINOR_TAG="30")				# 
add_definitions(-DVERSION_COPYRIGHT="March 2024")		# Up to here!

#windows
if(WIN32)
add_definitions(-D_WIN32_WINNT=0x0602)
add_definitions(-D_HAS_STD_BYTE=0)
endif()

# output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#UI IDs
add_definitions(-DID_START=5999)
add_definitions(-DID_NOISE_CANCEL=ID_START+21)
add_definitions(-DID_OCL=ID_NOISE_CANCEL+11)
add_definitions(-DID_TESTER=ID_OCL+51)
add_definitions(-DID_TRACE1=ID_TESTER+21)
add_definitions(-DID_TRACE2=ID_TRACE1+11)
add_definitions(-DID_VRENDER_VIEW1=ID_TRACE2+51)
add_definitions(-DID_SCRIPT_BREAK=ID_VRENDER_VIEW1+11)

# static compile
add_definitions(-DSTATIC_COMPILE)
add_definitions(-DLX_STATIC_LINKING)

# teem required definitions
add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)
add_definitions(-DTEEM_ZLIB=1)

# glm
add_definitions(-DGLM_FORCE_RADIANS=1)

# wxWidgets
set(wxWidgets_USE_REL_AND_DBG ON)
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
  set(wxWidgets_CONFIGURATION mswu)
elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  set(wxWidgets_CONFIGURATION mswud)
endif()
find_package(wxWidgets COMPONENTS core base aui html xml adv gl stc scintilla REQUIRED)
include(${wxWidgets_USE_FILE})
set(wxWidgets_USE_STATIC ON)
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
# solution for wxWidgets linking "ambiguous" errors
add_definitions(-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_)
# fix the library names for OSX if needed.
if(APPLE)
  foreach(arg ${wxWidgets_LIBRARIES})
    set(wxlib_out "${wxlib_out} ${arg}")
  endforeach()
  string(STRIP ${wxlib_out} wxWidgets_LIBRARIES)
  string(REGEX REPLACE "wx_osx_cocoau_scintilla" "wxscintilla"
    wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES})
elseif(UNIX OR MINGW)
  foreach(arg ${wxWidgets_LIBRARIES})
    set(wxlib_out "${wxlib_out} ${arg}")
  endforeach()
  string(STRIP ${wxlib_out} wxWidgets_LIBRARIES)
  string(REGEX REPLACE "wx_gtk3u_scintilla" "wxscintilla"
    wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES})
endif()
#copy wxwidgets patch to its source dir
#need to rebuild wxWidgets after this
if (WIN32)
file(COPY ${FluoRender_SOURCE_DIR}/wxWidgets/include/wx/aui/auibook.h
  DESTINATION ${wxWidgets_ROOT_DIR}/include/wx/aui)
file(COPY ${FluoRender_SOURCE_DIR}/wxWidgets/src/aui/auibook.cpp
  DESTINATION ${wxWidgets_ROOT_DIR}/src/aui)
endif()
# libtiff
if(WIN32)
    set(TIFF_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(TIFF_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(PNG_PNG_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/png)
    set(ZLIB_DIR ${wxWidgets_ROOT_DIR}/src/zlib)
    set(ZLIB_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/zlib)
    set(TIFF_LIBRARY_RELEASE ${wxWidgets_LIB_DIR}/wxtiff.lib)
    set(TIFF_LIBRARY_DEBUG ${wxWidgets_LIB_DIR}/wxtiffd.lib)
    set(PNG_LIBRARY_RELEASE ${wxWidgets_LIB_DIR}/wxpng.lib)
    set(PNG_LIBRARY_DEBUG ${wxWidgets_LIB_DIR}/wxpngd.lib)
    set(ZLIB_LIBRARY_RELEASE ${wxWidgets_LIB_DIR}/wxzlib.lib)
    set(ZLIB_LIBRARY_DEBUG ${wxWidgets_LIB_DIR}/wxzlibd.lib)
endif()
find_package(TIFF REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
if(WIN32)
    include_directories(${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    include_directories(${wxWidgets_ROOT_DIR}/src/png)
    include_directories(${wxWidgets_ROOT_DIR}/src/zlib)
else()
    include_directories(${TIFF_INCLUDE_DIR})
    if (TIFF_FOUND)
      message (STATUS "TIFF_INCLUDE_DIR=${TIFF_INCLUDE_DIR}")
    endif()
    include_directories(${PNG_PNG_INCLUDE_DIR})
    if (PNG_FOUND)
      message (STATUS "PNG_PNG_INCLUDE_DIR=${PNG_PNG_INCLUDE_DIR}")
    endif()
    include_directories(${ZLIB_INCLUDE_DIRS})
    if (ZLIB_FOUND)
      message (STATUS "ZLIB_INCLUDE_DIRS=${ZLIB_INCLUDE_DIRS}")
    endif()
endif()

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# Boost
# only find static libs:
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system chrono filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

#opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
if (OPENCV_FOUND)
  message (STATUS "OpenCV_INCLUDE_DIRS=${OpenCV_INCLUDE_DIRS}")
endif()

#python
#set(Python_ADDITIONAL_VERSIONS 3.10 3.9 3.8 3.7)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})
if (PYTHON3_FOUND)
  message (STATUS "Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS}")
endif()

#hdf5
if (WIN32 OR APPLE)
  set(HDF5_USE_STATIC_LIBRARIES ON)
  find_package(HDF5 REQUIRED)
  include_directories(${HDF5_INCLUDE_DIRS})
endif()

# JNI includes
find_package(JNI REQUIRED)
if (JNI_FOUND)
   message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
   message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
   include_directories(${JNI_INCLUDE_DIRS})
   #target_link_libraries(FluoRender ${JNI_LIBRARIES})
endif()

# FluoRender
include_directories(${FluoRender_SOURCE_DIR}/FluoRender)
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender)

# ffmpeg
if(WIN32)
  include_directories(${FluoRender_SOURCE_DIR}/FluoRender/ffmpeg/include)
  file(GLOB FFMPEG_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/ffmpeg/lib/Win64/*.a ${FluoRender_SOURCE_DIR}/FluoRender/ffmpeg/lib/Win64/*.lib)
  add_definitions(-D__STDC_CONSTANT_MACROS)
elseif(APPLE)
  include_directories(${FluoRender_SOURCE_DIR}/FluoRender/ffmpeg/include)
  file(GLOB FFMPEG_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/ffmpeg/lib/OSX/*.a)
else()
  find_path(FFMPEG_INCLUDE_DIR ffmpeg_include)
  include_directories(${FFMPEG_INCLUDE_DIR})
  find_library(AVCODEC_LIBRARY avcodec)
  find_library(AVFORMAT_LIBRARY avformat)
  find_library(AVUTIL_LIBRARY avutil)
  find_library(SWSCALE_LIBRARY swscale)
  find_library(SWRESAMPLE_LIBRARY swresample)
  set(
    FFMPEG_LIBRARIES
    ${AVFORMAT_LIBRARY}
    ${AVCODEC_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
    ${SWRESAMPLE_LIBRARY}
)
endif()

# freetype
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/freetype/include)
if(WIN32)
  file(GLOB FREETYPE_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/freetype/lib/*.lib)
else()
  find_package(Freetype REQUIRED)
  include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

# glew
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/glew)
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/glew/GL)
file(GLOB gl_srcs FluoRender/glew/*.c)
file(GLOB gl_hdrs1 FluoRender/glew/*.h)
file(GLOB gl_hdrs2 FluoRender/glew/GL/*.h)
add_library(GLEW_OBJ OBJECT
  ${gl_srcs} ${gl_hdrs1} ${gl_hdrs2})

# HoloPlay
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/include)
if(WIN32)
	if(${ARCHITECTURE} MATCHES 32)
		file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/Win32/*.lib)
    elseif(${ARCHITECTURE} MATCHES 64)
		file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/Win64/*.lib)
    endif()
elseif(APPLE)
	file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/macos/HoloPlayCore.dylib)
else()
	file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/linux/*.so)
endif()

#nikon
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/nd2/include)
if(WIN32)
  file(GLOB ND2_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/nd2/lib/*.lib)
else()
  file(GLOB ND2_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/nd2/lib/*.a)
endif()

# OpenCL
if(WIN32)
  include_directories(${FluoRender_SOURCE_DIR}/FluoRender/OpenCL/include)
  if(${ARCHITECTURE} MATCHES 64)
    file(GLOB OpenCL_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/OpenCL/lib/x86_64/*.lib)
  elseif(${ARCHITECTURE} MATCHES 32)
    file(GLOB OpenCL_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/OpenCL/lib/x86/*.lib)
  endif()
elseif(UNIX OR APPLE OR MINGW)
  #include(${CMAKE_SOURCE_DIR}/FindOpenCL.cmake)
  find_package(OpenCL REQUIRED)
  include_directories(${OpenCL_INCLUDE_DIRS})
endif()

# OpenVR
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/OpenVR/headers)
if(WIN32)
	if(${ARCHITECTURE} MATCHES 64)
		file(GLOB OpenVR_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/OpenVR/lib/win64/*.lib)
	endif()
endif()

# pole
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/pole)
file(GLOB pole_srcs FluoRender/pole/pole.cpp)
file(GLOB pole_hdrs FluoRender/pole/pole.h)
add_library(POLE_OBJ OBJECT ${pole_srcs} ${pole_hdrs})

# teem
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/teem)
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/teem/Hest)
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/teem/Nrrd)
file(GLOB airsrc FluoRender/teem/Air/*.c)
file(GLOB airhdr FluoRender/teem/Air/*.h)
file(GLOB hestsrc FluoRender/teem/Hest/*.c)
file(GLOB hesthdr FluoRender/teem/Hest/*.h)
file(GLOB biffsrc FluoRender/teem/Biff/*.c)
file(GLOB biffhdr FluoRender/teem/Biff/*.h)
file(GLOB nrrdsrc FluoRender/teem/Nrrd/*.c)
file(GLOB nrrdhdr FluoRender/teem/Nrrd/*.h)
add_library(TEEM_OBJ OBJECT
  ${airsrc} ${hestsrc} ${nrrdsrc} ${biffsrc}
  ${airhdr} ${hesthdr} ${nrrdhdr} ${biffhdr})

# XInput
if(WIN32 AND USE_XINPUT)
	include_directories(${FluoRender_SOURCE_DIR}/FluoRender/XInput)
	file(GLOB xip_src FluoRender/XInput/*.cpp)
	file(GLOB xip_hdr FluoRender/XInput/*.h)
	add_library(XINPUT_OBJ OBJECT
		${xip_src} ${xip_hdr})
endif()

# Animator
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Animator)
file(GLOB ani_src FluoRender/FluoRender/Animator/*.cpp)
file(GLOB ani_hdr FluoRender/FluoRender/Animator/*.h)
add_library(ANIMATORS_OBJ OBJECT
  ${ani_src} ${ani_hdr})

# Calculate
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Calculate)
file(GLOB calc_src FluoRender/FluoRender/Calculate/*.cpp)
file(GLOB calc_hdr FluoRender/FluoRender/Calculate/*.h)
add_library(CALCULATE_OBJ OBJECT
  ${calc_src} ${calc_hdr})

# Cluster
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Cluster)
file(GLOB clstr_src FluoRender/FluoRender/Cluster/*.cpp)
file(GLOB clstr_hdr FluoRender/FluoRender/Cluster/*.h)
add_library(CLUSTER_OBJ OBJECT
  ${clstr_src} ${clstr_hdr})

# Component
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Component)
file(GLOB cmp_src FluoRender/FluoRender/Component/*.cpp)
file(GLOB cmp_hdr FluoRender/FluoRender/Component/*.h)
add_library(COMPONENT_OBJ OBJECT
  ${cmp_src} ${cmp_hdr})

# Config
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Config)
file(GLOB config_src FluoRender/FluoRender/Config/*.cpp)
file(GLOB config_hdr FluoRender/FluoRender/Config/*.h)
add_library(CONFIG_OBJ OBJECT
  ${config_src} ${config_hdr})

# Control
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Control)
file(GLOB ctrl_src FluoRender/FluoRender/Control/*.cpp)
file(GLOB ctrl_hdr FluoRender/FluoRender/Control/*.h)
add_library(CONTROL_OBJ OBJECT
  ${ctrl_src} ${ctrl_hdr})

# Converter
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Converter)
file(GLOB cvt_src FluoRender/FluoRender/Converter/*.cpp)
file(GLOB cvt_hdr FluoRender/FluoRender/Converter/*.h)
add_library(CONVERTER_OBJ OBJECT
  ${cvt_src} ${cvt_hdr})

# Database
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Database)
file(GLOB dbs_src FluoRender/FluoRender/Database/*.cpp)
file(GLOB dbs_hdr FluoRender/FluoRender/Database/*.h)
add_library(DATABASE_OBJ OBJECT
  ${dbs_src} ${dbs_hdr})

# Distance
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Distance)
file(GLOB dist_src FluoRender/FluoRender/Distance/*.cpp)
file(GLOB dist_hdr FluoRender/FluoRender/Distance/*.h)
add_library(DISTANCE_OBJ OBJECT
  ${dist_src} ${dist_hdr})

# File
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/File)
file(GLOB file_src FluoRender/FluoRender/File/*.cpp)
file(GLOB file_hdr FluoRender/FluoRender/File/*.h)
add_library(FILE_OBJ OBJECT
  ${file_src} ${file_hdr})

# Filter
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Filter)
file(GLOB fltr_src FluoRender/FluoRender/Filter/*.cpp)
file(GLOB fltr_hdr FluoRender/FluoRender/Filter/*.h)
add_library(FILTER_OBJ OBJECT
  ${fltr_src} ${fltr_hdr})

# Flobject
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Flobject)
file(GLOB obj_src FluoRender/FluoRender/Flobject/*.cpp)
file(GLOB obj_hdr FluoRender/FluoRender/Flobject/*.hpp)
add_library(FLOBJECT_OBJ OBJECT
  ${obj_src} ${obj_hdr})

# Flrender
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Flrender)
file(GLOB flrd_src FluoRender/FluoRender/Flrender/*.cpp)
file(GLOB flrd_hdr FluoRender/FluoRender/Flrender/*.h)
add_library(FLRENDER_OBJ OBJECT
  ${flrd_src} ${flrd_hdr})

# Flui
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Flui)
file(GLOB flui_src FluoRender/FluoRender/Flui/*.cpp)
file(GLOB flui_hdr FluoRender/FluoRender/Flui/*.h)
add_library(FLUI_OBJ OBJECT
  ${flui_src} ${flui_hdr})

# Format
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Format)
file(GLOB fmt_src FluoRender/FluoRender/Format/*.cpp)
file(GLOB fmt_hdr FluoRender/FluoRender/Format/*.h)
file(GLOB fmt_rsc FluoRender/FluoRender/Format/*.class)
add_library(FORMAT_OBJ OBJECT
  ${fmt_src} ${fmt_hdr} ${fmt_rsc})

# Global
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Global)
file(GLOB glb_src FluoRender/FluoRender/Global/*.cpp)
file(GLOB glb_hdr FluoRender/FluoRender/Global/*.h)
add_library(GLOBAL_OBJ OBJECT
  ${glb_src} ${glb_hdr})

# Image
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Image)
file(GLOB img_hdr FluoRender/FluoRender/Image/*.h)
file(GLOB img_src FluoRender/FluoRender/Image/*.cpp)
file(GLOB img_rsc FluoRender/FluoRender/Image/*.xpm)
add_library(IMAGE_OBJ OBJECT
  ${img_hdr} ${img_src} ${img_rsc})

# LookingGlass
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/LookingGlass)
file(GLOB lgr_src FluoRender/FluoRender/LookingGlass/*.cpp)
file(GLOB lgr_hdr FluoRender/FluoRender/LookingGlass/*.h)
add_library(LOOKINGGLASS_OBJ OBJECT
  ${lgr_src} ${lgr_hdr})

# Project
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Project)
file(GLOB prj_src FluoRender/FluoRender/Project/*.cpp)
file(GLOB prj_hdr FluoRender/FluoRender/Project/*.h)
add_library(PROJECT_OBJ OBJECT
  ${prj_src} ${prj_hdr})

# Python
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Python)
file(GLOB pyth_hdr FluoRender/FluoRender/Python/*.h)
file(GLOB pyth_src FluoRender/FluoRender/Python/*.cpp)
add_library(PYTHON_OBJ OBJECT
  ${pyth_hdr} ${pyth_src})

# Scene
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Scene)
file(GLOB scn_src FluoRender/FluoRender/Scene/*.cpp)
file(GLOB scn_hdr FluoRender/FluoRender/Scene/*.h)
add_library(SCENE_OBJ OBJECT
  ${scn_src} ${scn_hdr})

# Script
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Script)
file(GLOB scr_src FluoRender/FluoRender/Script/*.cpp)
file(GLOB scr_hdr FluoRender/FluoRender/Script/*.h)
add_library(SCRIPT_OBJ OBJECT
  ${scr_src} ${scr_hdr})

# Selection
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Selection)
file(GLOB sel_src FluoRender/FluoRender/Selection/*.cpp)
file(GLOB sel_hdr FluoRender/FluoRender/Selection/*.h)
add_library(SELECTION_OBJ OBJECT
  ${sel_src} ${sel_hdr})

# Timer
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Timer)
file(GLOB timer_src FluoRender/FluoRender/Timer/*.cpp)
file(GLOB timer_hdr FluoRender/FluoRender/Timer/*.hpp)
add_library(TIMER_OBJ OBJECT
  ${timer_src} ${timer_hdr})

# Tracking
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Tracking)
file(GLOB trk_src FluoRender/FluoRender/Tracking/*.cpp)
file(GLOB trk_hdr FluoRender/FluoRender/Tracking/*.h)
add_library(TRACKING_OBJ OBJECT
  ${trk_src} ${trk_hdr})

# Type
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Type)
file(GLOB typ_src FluoRender/FluoRender/Type/*.cpp)
file(GLOB typ_hdr FluoRender/FluoRender/Type/*.h)
add_library(TYPE_OBJ OBJECT
  ${typ_src} ${typ_hdr})

# Utility
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Utility)
file(GLOB utl_src FluoRender/FluoRender/Utility/*.cpp)
file(GLOB utl_hdr FluoRender/FluoRender/Utility/*.h)
file(GLOB utl_rsc FluoRender/FluoRender/Utility/*.natvis)
add_library(UTILITY_OBJ OBJECT
  ${utl_src} ${utl_hdr} ${utl_rsc})

# Video
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Video)
file(GLOB vid_src FluoRender/FluoRender/Video/*.cpp)
file(GLOB vid_hdr FluoRender/FluoRender/Video/*.h)
add_library(VIDEO_OBJ OBJECT
  ${vid_src} ${vid_hdr})

# SynthBB
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/SynthBB)
file(GLOB synthbb_hdr FluoRender/SynthBB/*.h)
file(GLOB synthbb_src FluoRender/SynthBB/*.cpp)

# MaskInt
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/MaskInt)
file(GLOB maskint_hdr FluoRender/MaskInt/*.h)
file(GLOB maskint_src FluoRender/MaskInt/*.cpp)

# ReadGMM
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/ReadGMM)
file(GLOB readgmm_hdr FluoRender/ReadGMM/*.h)
file(GLOB readgmm_src FluoRender/ReadGMM/*.cpp)

# Tester
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/Tester)
file(GLOB tester_hdr FluoRender/Tester/*.h)
file(GLOB tester_src FluoRender/Tester/*.cpp)

# For Apple set the icns file containing icons
IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE FluoRender_icon.icns)
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/FluoRender/FluoRender/Image/FluoRender_icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(src ${src} ${CMAKE_CURRENT_SOURCE_DIR}/FluoRender/FluoRender/Image/FluoRender_icon.icns)
ELSEIF(WIN32)
  SET(icon_rc "${CMAKE_CURRENT_SOURCE_DIR}/FluoRender/FluoRender/Image/FluoRender.rc")
  SET(icns "${CMAKE_CURRENT_SOURCE_DIR}/FluoRender/FluoRender/Image/fluorenderIcon.ico")
  SET(manifest_file "${CMAKE_CURRENT_SOURCE_DIR}/Manifest/fluorender.manifest")
  ADD_DEFINITIONS(-DICON_RC_FILE="${icns}")
  SET(src ${src} ${icon_rc} ${manifest_file})
ENDIF()

# platform specific rules
if(APPLE)
  # Mac OS X
  if(NOT OPEN_GL_HEADER_LOC)
    add_definitions(-DOPEN_GL_HEADER_LOC=<OpenGL/gl.h>)
  endif()
  if(NOT OPEN_GLU_HEADER_LOC)
    add_definitions(-DOPEN_GLU_HEADER_LOC=<OpenGL/glu.h>)
  endif()
  add_definitions(-D_DARWIN)
  set(CFLAGS "-fPIC")
  set(CXXFLAGS "-fPIC ")
  include(BundleUtilities)
  add_executable(FluoRender MACOSX_BUNDLE
    ${src} ${hdr} ${rsc} ${icns}
    $<TARGET_OBJECTS:ANIMATORS_OBJ>
    $<TARGET_OBJECTS:CALCULATE_OBJ>
    $<TARGET_OBJECTS:CLUSTER_OBJ>
    $<TARGET_OBJECTS:COMPONENT_OBJ>
    $<TARGET_OBJECTS:CONFIG_OBJ>
    $<TARGET_OBJECTS:CONTROL_OBJ>
    $<TARGET_OBJECTS:CONVERTER_OBJ>
    $<TARGET_OBJECTS:DATABASE_OBJ>
    $<TARGET_OBJECTS:DISTANCE_OBJ>
    $<TARGET_OBJECTS:FILE_OBJ>
    $<TARGET_OBJECTS:FILTER_OBJ>
    $<TARGET_OBJECTS:FLRENDER_OBJ>
    $<TARGET_OBJECTS:FLOBJECT_OBJ>
    $<TARGET_OBJECTS:FLUI_OBJ>
    $<TARGET_OBJECTS:FORMAT_OBJ>
    $<TARGET_OBJECTS:GLOBAL_OBJ>
    $<TARGET_OBJECTS:IMAGE_OBJ>
    $<TARGET_OBJECTS:LOOKINGGLASS_OBJ>
    $<TARGET_OBJECTS:PROJECT_OBJ>
    $<TARGET_OBJECTS:PYTHON_OBJ>
    $<TARGET_OBJECTS:SCENE_OBJ>
    $<TARGET_OBJECTS:SCRIPT_OBJ>
    $<TARGET_OBJECTS:SELECTION_OBJ>
    $<TARGET_OBJECTS:TIMER_OBJ>
    $<TARGET_OBJECTS:TRACKING_OBJ>
    $<TARGET_OBJECTS:TYPE_OBJ>
    $<TARGET_OBJECTS:UTILITY_OBJ>
    $<TARGET_OBJECTS:VIDEO_OBJ>
    $<TARGET_OBJECTS:GLEW_OBJ>
    $<TARGET_OBJECTS:POLE_OBJ>
    $<TARGET_OBJECTS:TEEM_OBJ>
    )
elseif(UNIX OR MINGW)
  # Linux
  #if(NOT OPEN_GL_HEADER_LOC)
  #  add_definitions(-DOPEN_GL_HEADER_LOC=<OpenGL/gl.h>)
  #endif()
  #if(NOT OPEN_GLU_HEADER_LOC)
  #  add_definitions(-DOPEN_GLU_HEADER_LOC=<OpenGL/glu.h>)
  #endif()
  #add_definitions(-D_DARWIN)
  set(CFLAGS "-fPIC")
  set(CXXFLAGS "-fPIC ")
  #include(BundleUtilities)
  add_executable(FluoRender
    ${src} ${hdr} ${rsc} ${icns}
    $<TARGET_OBJECTS:ANIMATORS_OBJ>
    $<TARGET_OBJECTS:CALCULATE_OBJ>
    $<TARGET_OBJECTS:CLUSTER_OBJ>
    $<TARGET_OBJECTS:COMPONENT_OBJ>
    $<TARGET_OBJECTS:CONFIG_OBJ>
    $<TARGET_OBJECTS:CONTROL_OBJ>
    $<TARGET_OBJECTS:CONVERTER_OBJ>
    $<TARGET_OBJECTS:DATABASE_OBJ>
    $<TARGET_OBJECTS:DISTANCE_OBJ>
    $<TARGET_OBJECTS:FILE_OBJ>
    $<TARGET_OBJECTS:FILTER_OBJ>
    $<TARGET_OBJECTS:FLRENDER_OBJ>
    $<TARGET_OBJECTS:FLOBJECT_OBJ>
    $<TARGET_OBJECTS:FLUI_OBJ>
    $<TARGET_OBJECTS:FORMAT_OBJ>
    $<TARGET_OBJECTS:GLOBAL_OBJ>
    $<TARGET_OBJECTS:IMAGE_OBJ>
    $<TARGET_OBJECTS:LOOKINGGLASS_OBJ>
    $<TARGET_OBJECTS:PROJECT_OBJ>
    $<TARGET_OBJECTS:PYTHON_OBJ>
    $<TARGET_OBJECTS:SCENE_OBJ>
    $<TARGET_OBJECTS:SCRIPT_OBJ>
    $<TARGET_OBJECTS:SELECTION_OBJ>
    $<TARGET_OBJECTS:TIMER_OBJ>
    $<TARGET_OBJECTS:TRACKING_OBJ>
    $<TARGET_OBJECTS:TYPE_OBJ>
    $<TARGET_OBJECTS:UTILITY_OBJ>
    $<TARGET_OBJECTS:VIDEO_OBJ>
    $<TARGET_OBJECTS:GLEW_OBJ>
    $<TARGET_OBJECTS:POLE_OBJ>
    $<TARGET_OBJECTS:TEEM_OBJ>
    )
elseif(WIN32)
  # Windows
  if(NOT OPEN_GL_HEADER_LOC)
    add_definitions(-DOPEN_GL_HEADER_LOC=<GL/gl.h>)
  endif()
  if(NOT OPEN_GLU_HEADER_LOC)
    add_definitions(-DOPEN_GLU_HEADER_LOC=<GL/glu.h>)
  endif()
  # windows
  if(WIN32)
    add_definitions(-DWIN32)
    add_definitions(-D_WIN32)
    if(MSVC)
      #add_definitions(-DSEQAN_C++11_STANDARD=ON)
      add_definitions(/MP)
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
    include_directories(${FluoRender_SOURCE_DIR}/FluoRender/Wacom/Include)
    file(GLOB wac_src FluoRender/Wacom/*.cpp)
    file(GLOB wac_hdr FluoRender/Wacom/*.h)
    add_library(WACUTILS_OBJ OBJECT
      ${wac_src} ${wac_hdr})
    if (USE_XINPUT)
    add_executable(FluoRender WIN32
      ${src} ${hdr} ${rsc} ${icns}
      $<TARGET_OBJECTS:ANIMATORS_OBJ>
      $<TARGET_OBJECTS:CALCULATE_OBJ>
      $<TARGET_OBJECTS:CLUSTER_OBJ>
      $<TARGET_OBJECTS:COMPONENT_OBJ>
      $<TARGET_OBJECTS:CONFIG_OBJ>
      $<TARGET_OBJECTS:CONTROL_OBJ>
      $<TARGET_OBJECTS:CONVERTER_OBJ>
      $<TARGET_OBJECTS:DATABASE_OBJ>
      $<TARGET_OBJECTS:DISTANCE_OBJ>
      $<TARGET_OBJECTS:FILE_OBJ>
      $<TARGET_OBJECTS:FILTER_OBJ>
      $<TARGET_OBJECTS:FLRENDER_OBJ>
      $<TARGET_OBJECTS:FLOBJECT_OBJ>
      $<TARGET_OBJECTS:FLUI_OBJ>
      $<TARGET_OBJECTS:FORMAT_OBJ>
      $<TARGET_OBJECTS:GLOBAL_OBJ>
      $<TARGET_OBJECTS:IMAGE_OBJ>
      $<TARGET_OBJECTS:LOOKINGGLASS_OBJ>
      $<TARGET_OBJECTS:PROJECT_OBJ>
      $<TARGET_OBJECTS:PYTHON_OBJ>
      $<TARGET_OBJECTS:SCENE_OBJ>
      $<TARGET_OBJECTS:SCRIPT_OBJ>
      $<TARGET_OBJECTS:SELECTION_OBJ>
      $<TARGET_OBJECTS:TIMER_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:TYPE_OBJ>
      $<TARGET_OBJECTS:UTILITY_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:GLEW_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>
      $<TARGET_OBJECTS:WACUTILS_OBJ>
      $<TARGET_OBJECTS:XINPUT_OBJ>
    )
    else()
    add_executable(FluoRender WIN32
      ${src} ${hdr} ${rsc} ${icns}
      $<TARGET_OBJECTS:ANIMATORS_OBJ>
      $<TARGET_OBJECTS:CALCULATE_OBJ>
      $<TARGET_OBJECTS:CLUSTER_OBJ>
      $<TARGET_OBJECTS:COMPONENT_OBJ>
      $<TARGET_OBJECTS:CONFIG_OBJ>
      $<TARGET_OBJECTS:CONTROL_OBJ>
      $<TARGET_OBJECTS:CONVERTER_OBJ>
      $<TARGET_OBJECTS:DATABASE_OBJ>
      $<TARGET_OBJECTS:DISTANCE_OBJ>
      $<TARGET_OBJECTS:FILE_OBJ>
      $<TARGET_OBJECTS:FILTER_OBJ>
      $<TARGET_OBJECTS:FLRENDER_OBJ>
      $<TARGET_OBJECTS:FLOBJECT_OBJ>
      $<TARGET_OBJECTS:FLUI_OBJ>
      $<TARGET_OBJECTS:FORMAT_OBJ>
      $<TARGET_OBJECTS:GLOBAL_OBJ>
      $<TARGET_OBJECTS:IMAGE_OBJ>
      $<TARGET_OBJECTS:LOOKINGGLASS_OBJ>
      $<TARGET_OBJECTS:PROJECT_OBJ>
      $<TARGET_OBJECTS:PYTHON_OBJ>
      $<TARGET_OBJECTS:SCENE_OBJ>
      $<TARGET_OBJECTS:SCRIPT_OBJ>
      $<TARGET_OBJECTS:SELECTION_OBJ>
      $<TARGET_OBJECTS:TIMER_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:TYPE_OBJ>
      $<TARGET_OBJECTS:UTILITY_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:GLEW_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>
      $<TARGET_OBJECTS:WACUTILS_OBJ>
    )
    endif()
    add_executable(Tester
      ${tester_src} ${tester_hdr}
      $<TARGET_OBJECTS:ANIMATORS_OBJ>
      $<TARGET_OBJECTS:CALCULATE_OBJ>
      $<TARGET_OBJECTS:CLUSTER_OBJ>
      $<TARGET_OBJECTS:COMPONENT_OBJ>
      $<TARGET_OBJECTS:CONFIG_OBJ>
      $<TARGET_OBJECTS:CONTROL_OBJ>
      $<TARGET_OBJECTS:CONVERTER_OBJ>
      $<TARGET_OBJECTS:DATABASE_OBJ>
      $<TARGET_OBJECTS:DISTANCE_OBJ>
      $<TARGET_OBJECTS:FILE_OBJ>
      $<TARGET_OBJECTS:FILTER_OBJ>
      $<TARGET_OBJECTS:FLRENDER_OBJ>
      $<TARGET_OBJECTS:FLOBJECT_OBJ>
      $<TARGET_OBJECTS:FLUI_OBJ>
      $<TARGET_OBJECTS:FORMAT_OBJ>
      $<TARGET_OBJECTS:GLOBAL_OBJ>
      $<TARGET_OBJECTS:IMAGE_OBJ>
      $<TARGET_OBJECTS:LOOKINGGLASS_OBJ>
      $<TARGET_OBJECTS:PROJECT_OBJ>
      $<TARGET_OBJECTS:PYTHON_OBJ>
      $<TARGET_OBJECTS:SCENE_OBJ>
      $<TARGET_OBJECTS:SCRIPT_OBJ>
      $<TARGET_OBJECTS:SELECTION_OBJ>
      $<TARGET_OBJECTS:TIMER_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:TYPE_OBJ>
      $<TARGET_OBJECTS:UTILITY_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:GLEW_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>
      $<TARGET_OBJECTS:WACUTILS_OBJ>
      $<TARGET_OBJECTS:XINPUT_OBJ>
    )
  endif()
endif()

# architecture specific rules
if(${ARCHITECTURE} MATCHES 64)
  if(APPLE)
    set(ARCH_FLAGS "-m64 -arch x86_64")
  endif()
  add_definitions(-DTEEM_32BIT=0)
  set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}")
  set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
  set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
else()
  if(APPLE)
    set(ARCH_FLAGS "-m32 -arch i386")
  endif()
  add_definitions(-DTEEM_32BIT=1)
  set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}")
  set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
  set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
endif()

if(APPLE)
  set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/lib -L/usr/lib ${CMAKE_EXE_LINKER_FLAGS} -liconv -lbz2 -framework OpenCL -framework CoreFoundation -framework CoreVideo -framework VideoDecodeAcceleration -framework VideoToolbox -framework Security -framework CoreMedia")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.11 -DWX_PRECOMP -std=c++11 -stdlib=libc++")
elseif(UNIX OR MINGW)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -ldl")
endif()

target_compile_options(FluoRender PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(FluoRender PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")

# link the libraries
if(WIN32)
  target_link_libraries(FluoRender
    secur32.lib
    XInput.lib
    ${FFMPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${OpenVR_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ND2_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${OpenCV_LIBS}
    ${HoloPlay_LIBRARIES}
  )
  target_link_libraries(Tester
    secur32.lib
    XInput.lib
    ${FFMPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${OpenVR_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ND2_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${OpenCV_LIBS}
    ${HoloPlay_LIBRARIES}
  )
elseif(APPLE)
  target_link_libraries(FluoRender
    ${FFMPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ND2_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${OpenCV_LIBS}
    ${HoloPlay_LIBRARIES}
  )
else()
  target_link_libraries(FluoRender
    stdc++fs
    ${FFMPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ND2_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${OpenCV_LIBS}
    ${HoloPlay_LIBRARIES}
  )
endif()

# copy Java code dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Java_Code/ImageJ_Reader/target/classes"
  "$<TARGET_FILE_DIR:FluoRender>/Java_Code")
  
# copy openCL examples to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/CL_code"
  "$<TARGET_FILE_DIR:FluoRender>/CL_code")

# copy database dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Database"
  "$<TARGET_FILE_DIR:FluoRender>/Database")

# copy font dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Fonts"
  "$<TARGET_FILE_DIR:FluoRender>/Fonts")

# copy script dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Scripts"
  "$<TARGET_FILE_DIR:FluoRender>/Scripts")

# copy template dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Templates"
  "$<TARGET_FILE_DIR:FluoRender>/Templates")

# copy command dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Commands"
  "$<TARGET_FILE_DIR:FluoRender>/Commands")

# settings files
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Settings"
  "$<TARGET_FILE_DIR:FluoRender>")
if(APPLE)
	add_custom_command(TARGET FluoRender POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy
	  "${FluoRender_SOURCE_DIR}/Settings_Mac/fluorender.ini"
	  "$<TARGET_FILE_DIR:FluoRender>")
endif()

# copy data dir to the binary directory
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Data"
  "$<TARGET_FILE_DIR:FluoRender>/Data")

# benchmark files
add_custom_command(TARGET FluoRender POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Benchmark"
  "$<TARGET_FILE_DIR:FluoRender>")

# openvr
if(WIN32)
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/FluoRender/OpenVR/lib/win64/openvr_api.dll"
	"$<TARGET_FILE_DIR:FluoRender>")
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/Win64/HoloPlayCore.dll"
	"$<TARGET_FILE_DIR:FluoRender>")
endif()
