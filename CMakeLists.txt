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

# to build for Win7, disable xinput
SET(USE_XINPUT ON CACHE BOOL "Use Xinput on Windows")
if (USE_XINPUT)
  add_definitions(-DUSE_XINPUT=1)
endif()

cmake_minimum_required(VERSION 2.8.12)

# for MSVC builds
if(WIN32)
  if(MSVC)
    SET(MSVC_INCREMENTAL_DEFAULT OFF)
  endif()
endif()

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
add_definitions(-DVERSION_MINOR=29)						# 
add_definitions(-DVERSION_MAJOR_TAG="2")				# 
add_definitions(-DVERSION_MINOR_TAG="29")				# 
add_definitions(-DVERSION_COPYRIGHT="October 2022")		# Up to here!

#windows
if(WIN32)
add_definitions(-D_WIN32_WINNT=0x0602)
add_definitions(-D_HAS_STD_BYTE=0)
endif()

#UI IDs
add_definitions(-DID_START=5999)
add_definitions(-DID_ADJUST_VIEW=ID_START+1)
add_definitions(-DID_APROP_VIEW=ID_ADJUST_VIEW+31)
add_definitions(-DID_BRUSH_TOOL=ID_APROP_VIEW+11)
add_definitions(-DID_CALCULATION=ID_BRUSH_TOOL+51)
add_definitions(-DID_CLIP_VIEW=ID_CALCULATION+21)
add_definitions(-DID_COLOCALIZE=ID_CLIP_VIEW+41)
add_definitions(-DID_COMPONENT=ID_COLOCALIZE+11)
add_definitions(-DID_CONVERT=ID_COMPONENT+171)
add_definitions(-DID_COUNTING=ID_CONVERT+21)
add_definitions(-DID_LIST_PANEL1=ID_COUNTING+21)
add_definitions(-DID_LIST_PANEL2=ID_LIST_PANEL1+11)
add_definitions(-DID_MEASURE1=ID_LIST_PANEL2+11)
add_definitions(-DID_MEASURE2=ID_MEASURE1+11)
add_definitions(-DID_MANIPULATOR=ID_MEASURE2+21)
add_definitions(-DID_MPROP_VIEW=ID_MANIPULATOR+21)
add_definitions(-DID_NOISE_CANCEL=ID_MPROP_VIEW+21)
add_definitions(-DID_OCL=ID_NOISE_CANCEL+11)
add_definitions(-DID_RECORD1=ID_OCL+11)
add_definitions(-DID_RECORD2=ID_RECORD1+11)
add_definitions(-DID_SETTING=ID_RECORD2+11)
add_definitions(-DID_TESTER=ID_SETTING+51)
add_definitions(-DID_TRACE1=ID_TESTER+21)
add_definitions(-DID_TRACE2=ID_TRACE1+11)
add_definitions(-DID_TREE_PANEL1=ID_TRACE2+81)
add_definitions(-DID_TREE_PANEL2=ID_TREE_PANEL1+31)
add_definitions(-DID_MOVIEW_VIEW=ID_TREE_PANEL2+11)
add_definitions(-DID_VPROP_VIEW=ID_MOVIEW_VIEW+61)
add_definitions(-DID_VRENDER_FRAME=ID_VPROP_VIEW+71)
add_definitions(-DID_VRENDER_VIEW1=ID_VRENDER_FRAME+51)
add_definitions(-DID_VRENDER_VIEW2=ID_VRENDER_VIEW1+21)
add_definitions(-DID_LEARNING=ID_VRENDER_VIEW2+51)
add_definitions(-DID_LEARNING_COMP_GEN=ID_LEARNING+11)
add_definitions(-DID_LEARNING_VOL_PROP=ID_LEARNING_COMP_GEN+11)
#dialog common IDs
add_definitions(-DID_CROP=ID_LEARNING_VOL_PROP+11)
add_definitions(-DID_LZW_COMP=ID_CROP+1)
add_definitions(-DID_RESIZE_CHK=ID_LZW_COMP+1)
add_definitions(-DID_RESIZE_X_TXT=ID_RESIZE_CHK+1)
add_definitions(-DID_RESIZE_Y_TXT=ID_RESIZE_X_TXT+1)
add_definitions(-DID_RESIZE_Z_TXT=ID_RESIZE_Y_TXT+1)
add_definitions(-DID_SAVE_ALPHA=ID_RESIZE_Z_TXT+1)
add_definitions(-DID_FILTER=ID_SAVE_ALPHA+1)
add_definitions(-DID_EMBED_FILES=ID_FILTER+1)
add_definitions(-DID_READ_ZSLICE=ID_EMBED_FILES+1)
add_definitions(-DID_READ_CHANN=ID_READ_ZSLICE+1)
add_definitions(-DID_DIGI_ORDER=ID_READ_CHANN+1)
add_definitions(-DID_COMPRESS=ID_DIGI_ORDER+1)
add_definitions(-DID_SKIP_BRICKS=ID_COMPRESS+1)
add_definitions(-DID_TSEQ_ID=ID_SKIP_BRICKS+1)
add_definitions(-DID_ENLARGE_SLDR=ID_TSEQ_ID+1)
add_definitions(-DID_ENLARGE_TEXT=ID_ENLARGE_SLDR+1)
add_definitions(-DID_ENLARGE_CHK=ID_ENLARGE_TEXT+1)
add_definitions(-DID_SAVE_FLOAT=ID_ENLARGE_CHK+1)
add_definitions(-DID_DPI=ID_SAVE_FLOAT+1)
add_definitions(-DID_SER_NUM=ID_DPI+1)

# static compile
add_definitions(-DSTATIC_COMPILE)

# teem required definitions
add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

# glm
add_definitions(-DGLM_FORCE_RADIANS=1)

# output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
if(UNIX OR APPLE OR MINGW)
  find_package(TIFF REQUIRED)
  include_directories(${TIFF_INCLUDE_DIR})
  find_package(PNG REQUIRED)
  include_directories(${PNG_INCLUDE_DIR})
else()
  include_directories(${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
endif()
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

# OpenGL
find_package(OpenGL REQUIRED)

# Boost
# only find static libs:
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system chrono filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# JNI includes
find_package(JNI REQUIRED)
if (JNI_FOUND)
   message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
   message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
   include_directories(${JNI_INCLUDE_DIRS})
   #target_link_libraries(FluoRender ${JNI_LIBRARIES})
endif()

# teem
# add zlib
find_package(ZLIB)
if(ZLIB_FOUND)
  add_definitions(-DTEEM_ZLIB=1)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
file(GLOB airsrc fluorender/teem/Air/*.c)
file(GLOB airhdr fluorender/teem/Air/*.h)
file(GLOB hestsrc fluorender/teem/Hest/*.c)
file(GLOB hesthdr fluorender/teem/Hest/*.h)
file(GLOB biffsrc fluorender/teem/Biff/*.c)
file(GLOB biffhdr fluorender/teem/Biff/*.h)
file(GLOB nrrdsrc fluorender/teem/Nrrd/*.c)
file(GLOB nrrdhdr fluorender/teem/Nrrd/*.h)
add_library(TEEM_OBJ OBJECT
  ${airsrc} ${hestsrc} ${nrrdsrc} ${biffsrc}
  ${airhdr} ${hesthdr} ${nrrdhdr} ${biffhdr})

#eigen
#include_directories(${FluoRender_SOURCE_DIR}/fluorender/Eigen)

# glew
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
file(GLOB gl_srcs fluorender/glew/*.c)
file(GLOB gl_hdrs1 fluorender/glew/*.h)
file(GLOB gl_hdrs2 fluorender/glew/GL/*.h)
add_library(GLEW_OBJ OBJECT
  ${gl_srcs} ${gl_hdrs1} ${gl_hdrs2})

# pole
include_directories(${FluoRender_SOURCE_DIR}/fluorender/pole)
file(GLOB pole_srcs fluorender/pole/pole.cpp)
file(GLOB pole_hdrs fluorender/pole/pole.h)
add_library(POLE_OBJ OBJECT ${pole_srcs} ${pole_hdrs})

# ffmpeg
if(WIN32)
  include_directories(${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/include)
  file(GLOB FFMPEG_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/lib/Win64/*.a ${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/lib/Win64/*.lib)
  add_definitions(-D__STDC_CONSTANT_MACROS)
elseif(APPLE)
  include_directories(${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/include)
  file(GLOB FFMPEG_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/lib/OSX/*.a)
else()
  find_package(FFMPEG REQUIRED)
  #string(REPLACE "/usr/include/" "" FFMPEG_INCLUDE_DIR "${FFMPEG_INCLUDE_DIR}")
  include_directories(${FFMPEG_INCLUDE_DIR})
endif()

# FluoRender
include_directories(${FluoRender_SOURCE_DIR}/fluorender)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender)

# Global
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Global)
file(GLOB glb_src fluorender/FluoRender/Global/*.cpp)
file(GLOB glb_hdr fluorender/FluoRender/Global/*.h)
add_library(GLOBAL_OBJ OBJECT
  ${glb_src} ${glb_hdr})

# Formats
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Formats)
file(GLOB fmt_src fluorender/FluoRender/Formats/*.cpp)
file(GLOB fmt_hdr fluorender/FluoRender/Formats/*.h)
file(GLOB fmt_rsc fluorender/FluoRender/Formats/*.class)
add_library(FORMATS_OBJ OBJECT
  ${fmt_src} ${fmt_hdr} ${fmt_rsc})

# Video
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Video)
file(GLOB vid_src fluorender/FluoRender/Video/*.cpp)
file(GLOB vid_hdr fluorender/FluoRender/Video/*.h)
add_library(VIDEO_OBJ OBJECT
  ${vid_src} ${vid_hdr})

# Tracking
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Tracking)
file(GLOB trk_src fluorender/FluoRender/Tracking/*.cpp)
file(GLOB trk_hdr fluorender/FluoRender/Tracking/*.h)
add_library(TRACKING_OBJ OBJECT
  ${trk_src} ${trk_hdr})

# Components
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Components)
file(GLOB cmp_src fluorender/FluoRender/Components/*.cpp)
file(GLOB cmp_hdr fluorender/FluoRender/Components/*.h)
add_library(COMPONENTS_OBJ OBJECT
  ${cmp_src} ${cmp_hdr})

# Selection
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Selection)
file(GLOB sel_src fluorender/FluoRender/Selection/*.cpp)
file(GLOB sel_hdr fluorender/FluoRender/Selection/*.h)
add_library(SELECTION_OBJ OBJECT
  ${sel_src} ${sel_hdr})

# Script
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Script)
file(GLOB scr_src fluorender/FluoRender/Script/*.cpp)
file(GLOB scr_hdr fluorender/FluoRender/Script/*.h)
add_library(SCRIPT_OBJ OBJECT
  ${scr_src} ${scr_hdr})

# Cluster
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Cluster)
file(GLOB clstr_src fluorender/FluoRender/Cluster/*.cpp)
file(GLOB clstr_hdr fluorender/FluoRender/Cluster/*.h)
add_library(CLUSTER_OBJ OBJECT
  ${clstr_src} ${clstr_hdr})

# Distance
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Distance)
file(GLOB dist_src fluorender/FluoRender/Distance/*.cpp)
file(GLOB dist_hdr fluorender/FluoRender/Distance/*.h)
add_library(DISTANCE_OBJ OBJECT
  ${dist_src} ${dist_hdr})

# Calculate
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Calculate)
file(GLOB calc_src fluorender/FluoRender/Calculate/*.cpp)
file(GLOB calc_hdr fluorender/FluoRender/Calculate/*.h)
add_library(CALCULATE_OBJ OBJECT
  ${calc_src} ${calc_hdr})

# Database
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Database)
file(GLOB dbs_src fluorender/FluoRender/Database/*.cpp)
file(GLOB dbs_hdr fluorender/FluoRender/Database/*.h)
add_library(DATABASE_OBJ OBJECT
  ${dbs_src} ${dbs_hdr})

# Animators
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Animators)
file(GLOB ani_src fluorender/FluoRender/Animator/*.cpp)
file(GLOB ani_hdr fluorender/FluoRender/Animator/*.h)
add_library(ANIMATORS_OBJ OBJECT
  ${ani_src} ${ani_hdr})

# Flobject
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Flobject)
file(GLOB obj_src fluorender/FluoRender/Flobject/*.cpp)
file(GLOB obj_hdr fluorender/FluoRender/Flobject/*.hpp)
add_library(FLOBJECT_OBJ OBJECT
  ${obj_src} ${obj_hdr})

# FileIO
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/FileIO)
file(GLOB file_src fluorender/FluoRender/FileIO/*.cpp)
file(GLOB file_hdr fluorender/FluoRender/FileIO/*.h)
add_library(FILEIO_OBJ OBJECT
  ${file_src} ${file_hdr})

# Types
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Types)
file(GLOB typ_src fluorender/FluoRender/Types/*.cpp)
file(GLOB typ_hdr fluorender/FluoRender/Types/*.h)
add_library(TYPES_OBJ OBJECT
  ${typ_src} ${typ_hdr})

# Timer
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Timer)
file(GLOB timer_src fluorender/FluoRender/Timer/*.cpp)
file(GLOB timer_hdr fluorender/FluoRender/Timer/*.h)
add_library(TIMER_OBJ OBJECT
  ${timer_src} ${timer_hdr})

# FLIVR
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/FLIVR)
file(GLOB fli_src fluorender/FluoRender/FLIVR/*.cpp)
file(GLOB fli_hdr fluorender/FluoRender/FLIVR/*.h)
add_library(FLIVR_OBJ OBJECT
  ${fli_src} ${fli_hdr})

# Converters
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Converters)
file(GLOB cvt_src fluorender/FluoRender/Converters/*.cpp)
file(GLOB cvt_hdr fluorender/FluoRender/Converters/*.h)
add_library(CONVERTERS_OBJ OBJECT
  ${cvt_src} ${cvt_hdr})

# images
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/img)
file(GLOB pngs_hdr fluorender/FluoRender/img/*.h)
file(GLOB pngs_src fluorender/FluoRender/img/*.cpp)
add_library(IMAGES_OBJ OBJECT  ${pngs_hdr} ${pngs_src})

# OpenCL
if(WIN32)
  include_directories(${FluoRender_SOURCE_DIR}/fluorender/OpenCL/include)
  if(${ARCHITECTURE} MATCHES 64)
    file(GLOB OpenCL_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/OpenCL/lib/x86_64/*.lib)
  elseif(${ARCHITECTURE} MATCHES 32)
    file(GLOB OpenCL_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/OpenCL/lib/x86/*.lib)
  endif()
elseif(UNIX OR APPLE OR MINGW)
  #include(${CMAKE_SOURCE_DIR}/FindOpenCL.cmake)
  find_package(OpenCL REQUIRED)
  include_directories(${OpenCL_INCLUDE_DIRS})
endif()

#nikon
include_directories(${FluoRender_SOURCE_DIR}/fluorender/nd2/include)
if(WIN32)
	if(${ARCHITECTURE} MATCHES 64)
		file(GLOB ND2_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/nd2/lib/*.lib)
	endif()
elseif(APPLE)
   file(GLOB ND2_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/nd2/lib/*.dylib)
endif()

# freetype
include_directories(${FluoRender_SOURCE_DIR}/fluorender/freetype/include)
if(WIN32)
  file(GLOB FREETYPE_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/freetype/lib/*.lib)
else()
  find_package(Freetype REQUIRED)
  include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

# OpenVR
include_directories(${FluoRender_SOURCE_DIR}/fluorender/OpenVR/headers)
if(WIN32)
	if(${ARCHITECTURE} MATCHES 64)
		file(GLOB OpenVR_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/OpenVR/lib/win64/*.lib)
	endif()
endif()

#XInput
if(WIN32 AND USE_XINPUT)
	include_directories(${FluoRender_SOURCE_DIR}/fluorender/XInput)
	file(GLOB xip_src fluorender/XInput/*.cpp)
	file(GLOB xip_hdr fluorender/XInput/*.h)
	add_library(XINPUT_OBJ OBJECT
		${xip_src} ${xip_hdr})
endif()

# other sources
file(GLOB src fluorender/FluoRender/*.cpp)
file(GLOB hdr fluorender/FluoRender/*.h)
file(GLOB rsc fluorender/FluoRender/img/*.xpm)

# SynthBB
include_directories(${FluoRender_SOURCE_DIR}/fluorender/SynthBB)
file(GLOB synthbb_hdr fluorender/SynthBB/*.h)
file(GLOB synthbb_src fluorender/SynthBB/*.cpp)

# MaskInt
include_directories(${FluoRender_SOURCE_DIR}/fluorender/MaskInt)
file(GLOB maskint_hdr fluorender/MaskInt/*.h)
file(GLOB maskint_src fluorender/MaskInt/*.cpp)

# ReadGMM
include_directories(${FluoRender_SOURCE_DIR}/fluorender/ReadGMM)
file(GLOB readgmm_hdr fluorender/ReadGMM/*.h)
file(GLOB readgmm_src fluorender/ReadGMM/*.cpp)

# Tester
include_directories(${FluoRender_SOURCE_DIR}/fluorender/Tester)
file(GLOB tester_hdr fluorender/Tester/*.h)
file(GLOB tester_src fluorender/Tester/*.cpp)

# For Apple set the icns file containing icons
IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE FluoRender_icon.icns)
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/fluorender/FluoRender/img/FluoRender_icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(src ${src} ${CMAKE_CURRENT_SOURCE_DIR}/fluorender/FluoRender/img/FluoRender_icon.icns)
ELSEIF(WIN32)
  SET(icon_rc "${CMAKE_CURRENT_SOURCE_DIR}/fluorender/FluoRender/img/FluoRender.rc")
  SET(icns "${CMAKE_CURRENT_SOURCE_DIR}/fluorender/FluoRender/img/fluorenderIcon.ico")
  ADD_DEFINITIONS(-DICON_RC_FILE="${icns}")
  SET(src ${src} ${icon_rc})
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
    $<TARGET_OBJECTS:GLOBAL_OBJ>
    $<TARGET_OBJECTS:IMAGES_OBJ>
    $<TARGET_OBJECTS:CONVERTERS_OBJ>
    $<TARGET_OBJECTS:FLIVR_OBJ>
    $<TARGET_OBJECTS:ANIMATORS_OBJ>
    $<TARGET_OBJECTS:TYPES_OBJ>
    $<TARGET_OBJECTS:FLOBJECT_OBJ>
    $<TARGET_OBJECTS:FILEIO_OBJ>
    $<TARGET_OBJECTS:TIMER_OBJ>
    $<TARGET_OBJECTS:FORMATS_OBJ>
    $<TARGET_OBJECTS:VIDEO_OBJ>
    $<TARGET_OBJECTS:TRACKING_OBJ>
    $<TARGET_OBJECTS:COMPONENTS_OBJ>
    $<TARGET_OBJECTS:SELECTION_OBJ>
    $<TARGET_OBJECTS:SCRIPT_OBJ>
    $<TARGET_OBJECTS:CLUSTER_OBJ>
    $<TARGET_OBJECTS:DISTANCE_OBJ>
    $<TARGET_OBJECTS:CALCULATE_OBJ>
    $<TARGET_OBJECTS:DATABASE_OBJ>
    $<TARGET_OBJECTS:GLEW_OBJ>
    $<TARGET_OBJECTS:POLE_OBJ>
    $<TARGET_OBJECTS:TEEM_OBJ>)
  add_executable(Tester MACOSX_BUNDLE
    ${tester_src} ${tester_hdr}
    $<TARGET_OBJECTS:TYPES_OBJ>
    $<TARGET_OBJECTS:FLOBJECT_OBJ>
    $<TARGET_OBJECTS:FILEIO_OBJ>
    $<TARGET_OBJECTS:DATABASE_OBJ>)
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
    $<TARGET_OBJECTS:GLOBAL_OBJ>
    $<TARGET_OBJECTS:IMAGES_OBJ>
    $<TARGET_OBJECTS:CONVERTERS_OBJ>
    $<TARGET_OBJECTS:FLIVR_OBJ>
    $<TARGET_OBJECTS:ANIMATORS_OBJ>
    $<TARGET_OBJECTS:TYPES_OBJ>
    $<TARGET_OBJECTS:FLOBJECT_OBJ>
    $<TARGET_OBJECTS:FILEIO_OBJ>
    $<TARGET_OBJECTS:TIMER_OBJ>
    $<TARGET_OBJECTS:FORMATS_OBJ>
    $<TARGET_OBJECTS:VIDEO_OBJ>
    $<TARGET_OBJECTS:TRACKING_OBJ>
    $<TARGET_OBJECTS:COMPONENTS_OBJ>
    $<TARGET_OBJECTS:SELECTION_OBJ>
    $<TARGET_OBJECTS:SCRIPT_OBJ>
    $<TARGET_OBJECTS:CLUSTER_OBJ>
    $<TARGET_OBJECTS:DISTANCE_OBJ>
    $<TARGET_OBJECTS:CALCULATE_OBJ>
    $<TARGET_OBJECTS:DATABASE_OBJ>
    $<TARGET_OBJECTS:GLEW_OBJ>
    $<TARGET_OBJECTS:POLE_OBJ>
    $<TARGET_OBJECTS:TEEM_OBJ>)
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
    include_directories(${FluoRender_SOURCE_DIR}/fluorender/Wacom/Include)
    file(GLOB wac_src fluorender/Wacom/*.cpp)
    file(GLOB wac_hdr fluorender/Wacom/*.h)
    add_library(WACUTILS_OBJ OBJECT
      ${wac_src} ${wac_hdr})
    if (USE_XINPUT)
    add_executable(FluoRender WIN32
      ${src} ${hdr} ${rsc} ${icns}
      $<TARGET_OBJECTS:GLOBAL_OBJ>
      $<TARGET_OBJECTS:TYPES_OBJ>
      $<TARGET_OBJECTS:FLOBJECT_OBJ>
      $<TARGET_OBJECTS:FILEIO_OBJ>
      $<TARGET_OBJECTS:IMAGES_OBJ>
      $<TARGET_OBJECTS:CONVERTERS_OBJ>
      $<TARGET_OBJECTS:FLIVR_OBJ>
      $<TARGET_OBJECTS:ANIMATORS_OBJ>
      $<TARGET_OBJECTS:TIMER_OBJ>
      $<TARGET_OBJECTS:FORMATS_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:COMPONENTS_OBJ>
      $<TARGET_OBJECTS:SELECTION_OBJ>
      $<TARGET_OBJECTS:SCRIPT_OBJ>
      $<TARGET_OBJECTS:CLUSTER_OBJ>
      $<TARGET_OBJECTS:DISTANCE_OBJ>
      $<TARGET_OBJECTS:CALCULATE_OBJ>
      $<TARGET_OBJECTS:DATABASE_OBJ>
      $<TARGET_OBJECTS:GLEW_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>
      $<TARGET_OBJECTS:WACUTILS_OBJ>
      $<TARGET_OBJECTS:XINPUT_OBJ>
      )
    else()
    add_executable(FluoRender WIN32
      ${src} ${hdr} ${rsc} ${icns}
      $<TARGET_OBJECTS:GLOBAL_OBJ>
      $<TARGET_OBJECTS:TYPES_OBJ>
      $<TARGET_OBJECTS:FLOBJECT_OBJ>
      $<TARGET_OBJECTS:FILEIO_OBJ>
      $<TARGET_OBJECTS:IMAGES_OBJ>
      $<TARGET_OBJECTS:CONVERTERS_OBJ>
      $<TARGET_OBJECTS:FLIVR_OBJ>
      $<TARGET_OBJECTS:ANIMATORS_OBJ>
      $<TARGET_OBJECTS:TIMER_OBJ>
      $<TARGET_OBJECTS:FORMATS_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:COMPONENTS_OBJ>
      $<TARGET_OBJECTS:SELECTION_OBJ>
      $<TARGET_OBJECTS:SCRIPT_OBJ>
      $<TARGET_OBJECTS:CLUSTER_OBJ>
      $<TARGET_OBJECTS:DISTANCE_OBJ>
      $<TARGET_OBJECTS:CALCULATE_OBJ>
      $<TARGET_OBJECTS:DATABASE_OBJ>
      $<TARGET_OBJECTS:GLEW_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>
      $<TARGET_OBJECTS:WACUTILS_OBJ>
      )
    endif()
    add_executable(Tester
      ${tester_src} ${tester_hdr}
      $<TARGET_OBJECTS:GLOBAL_OBJ>
      $<TARGET_OBJECTS:TYPES_OBJ>
      $<TARGET_OBJECTS:FLOBJECT_OBJ>
      $<TARGET_OBJECTS:FILEIO_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:DATABASE_OBJ>)
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
  # make sure png is static linked
  string(REGEX REPLACE "-lpng" "/usr/local/lib/libpng.a"
    wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES})
  string(REGEX REPLACE "-lpng" "/usr/local/lib/libpng.a"
    CMAKE_MAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
  string(REGEX REPLACE "/usr/local/lib/libfreetype.dylib" "/usr/local/lib/libfreetype.a;/usr/local/lib/libpng.a"
    FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES})
  message(STATUS "${FREETYPE_LIBRARIES}")
elseif(UNIX OR MINGW)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -ldl")
endif()

target_compile_options(FluoRender PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(FluoRender PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")

# link the libraries
if(UNIX OR APPLE OR MINGW)
  target_link_libraries(FluoRender
    ${FFMPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ND2_LIBRARIES})
  target_link_libraries(Tester
    ${FFMPEG_LIBRARIES})
elseif(WIN32)
  target_link_libraries(FluoRender
    secur32.lib
    XInput.lib
    ${FFMPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${OpenVR_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    ${ND2_LIBRARIES})
  target_link_libraries(Tester
    secur32.lib
    ${FFMPEG_LIBRARIES})
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
	  "${FluoRender_SOURCE_DIR}/Settings_Mac/fluorender.set"
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
	"${FluoRender_SOURCE_DIR}/fluorender/OpenVR/lib/win64/openvr_api.dll"
	"$<TARGET_FILE_DIR:FluoRender>")
endif()

# nd2
if(WIN32)
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/fluorender/nd2/bin/limfile.dll"
	"$<TARGET_FILE_DIR:FluoRender>"
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/fluorender/nd2/bin/nd2readsdk.dll"
	"$<TARGET_FILE_DIR:FluoRender>"
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/fluorender/nd2/bin/tiff.dll"
	"$<TARGET_FILE_DIR:FluoRender>"
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/fluorender/nd2/bin/zlib1.dll"
	"$<TARGET_FILE_DIR:FluoRender>"
	)
elseif(APPLE)
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/fluorender/nd2/lib/libNd2File.dylib"
	"$<TARGET_FILE_DIR:FluoRender>"
	COMMAND ${CMAKE_COMMAND} -E copy
	"${FluoRender_SOURCE_DIR}/fluorender/nd2/lib/libNd2ReadSdk.dylib"
	"$<TARGET_FILE_DIR:FluoRender>"
	)
add_custom_command(TARGET FluoRender POST_BUILD COMMAND 
    ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/"
    $<TARGET_FILE:FluoRender>)
endif()

# make the Executable dir
if(WIN32)
  add_custom_target(make_exe_dir ALL 
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:FluoRender>/Executables")
else()
  add_custom_command(TARGET FluoRender POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:FluoRender>/Executables")
endif()
