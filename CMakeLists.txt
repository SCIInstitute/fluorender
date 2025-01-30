# /*
# For more information, please see: http://software.sci.utah.edu
# 
# The MIT License
# 
# Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

if (WIN32)
# to build for Win7, disable xinput
  SET(USE_XINPUT ON CACHE BOOL "Use Xinput on Windows")
  if (USE_XINPUT)
    add_definitions(-DUSE_XINPUT=1)
  endif()
# option to build with multithreading
  SET(USE_MP_COMPILE ON CACHE BOOL "Enable Multithreaded Compile")
# option to build tester
  SET(BUILD_TESTER OFF CACHE BOOL "Enable building the tester")
endif()

cmake_minimum_required(VERSION 3.10)

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

if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.5" CACHE STRING "Minimum OS X deployment version")
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
add_definitions(-DVERSION_MINOR=32)						# 
add_definitions(-DVERSION_MAJOR_TAG="2")				# 
add_definitions(-DVERSION_MINOR_TAG="32")				# 
add_definitions(-DVERSION_COPYRIGHT="January 2025")	    # Up to here!

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

cmake_path(GET FluoRender_SOURCE_DIR PARENT_PATH PARENT_DIR)

# static compile
add_definitions(-DSTATIC_COMPILE)
add_definitions(-DLX_STATIC_LINKING)

# glm
add_definitions(-DGLM_FORCE_RADIANS=1)

# Set the GLEW directory
if(WIN32 OR APPLE)
    file(GLOB GLEW_DIRS "${PARENT_DIR}/glew*")
    list(GET GLEW_DIRS 0 GLEW_DIR)  # Get the first match
    set(GLEW_INCLUDE_DIR "${GLEW_DIR}/include")
endif()
if(WIN32)
    set(GLEW_SHARED_LIBRARY_DEBUG "${GLEW_DIR}/lib/Debug/x64/glew32d.lib")
    set(GLEW_SHARED_LIBRARY_RELEASE "${GLEW_DIR}/lib/Release/x64/glew32.lib")
    set(GLEW_STATIC_LIBRARY_DEBUG "${GLEW_DIR}/lib/Debug/x64/glew32sd.lib")
    set(GLEW_STATIC_LIBRARY_RELEASE "${GLEW_DIR}/lib/Release/x64/glew32s.lib")
elseif(APPLE)
    set(GLEW_SHARED_LIBRARY_DEBUG "${GLEW_DIR}/lib/libGLEW.dylib")
    set(GLEW_SHARED_LIBRARY_RELEASE "${GLEW_DIR}/lib/libGLEW.dylib")
    set(GLEW_STATIC_LIBRARY_DEBUG "${GLEW_DIR}/lib/libGLEW.a")
    set(GLEW_STATIC_LIBRARY_RELEASE "${GLEW_DIR}/lib/libGLEW.a")
endif()
add_definitions(-DGLEW_STATIC)
set(GLEW_USE_STATIC_LIBS TRUE)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# wxWidgets
file(GLOB WXWIDGETS_DIRS "${PARENT_DIR}/wxWidgets*")
list(GET WXWIDGETS_DIRS 0 WXWIDGETS_DIR)  # Get the first match
message(STATUS "wxWidgets directory: ${WXWIDGETS_DIR}")
if(WIN32)
    # Windows-specific settings
    set(wxWidgets_ROOT_DIR ${WXWIDGETS_DIR})
    set(wxWidgets_LIB_DIR ${WXWIDGETS_DIR}/lib/vc_x64_lib)
    set(wxWidgets_INCLUDE_DIRS ${WXWIDGETS_DIR}/include)
else()
    set(wxWidgets_ROOT_DIR ${WXWIDGETS_DIR})
    set(wxWidgets_CONFIG_EXECUTABLE ${WXWIDGETS_DIR}/mybuild/wx-config)
endif()
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
file(COPY ${FluoRender_SOURCE_DIR}/wxWidgets/include/wx/aui/auibar.h
  DESTINATION ${wxWidgets_ROOT_DIR}/include/wx/aui)
file(COPY ${FluoRender_SOURCE_DIR}/wxWidgets/src/aui/auibook.cpp
  DESTINATION ${wxWidgets_ROOT_DIR}/src/aui)
file(COPY ${FluoRender_SOURCE_DIR}/wxWidgets/src/aui/auibar.cpp
  DESTINATION ${wxWidgets_ROOT_DIR}/src/aui)
endif()
# libtiff
if(WIN32)
    set(Tiff_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(TIFF_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(PNG_PNG_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/png)
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
    set(Tiff_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(ZLIB_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/zlib)
    set(TIFF_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
endif()
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

# Boost
file(GLOB BOOST_DIRS "${PARENT_DIR}/boost*")
list(GET BOOST_DIRS 0 BOOST_DIRS)  # Get the first match
set(Boost_INCLUDE_DIR "${BOOST_DIRS}")
# only find static libs:
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system chrono filesystem locale REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# dlib
file(GLOB DLIB_DIRS "${PARENT_DIR}/dlib*")
list(GET DLIB_DIRS 0 DLIB_DIRS)  # Get the first match
add_subdirectory(${DLIB_DIRS} dlib_build)
include_directories(${DLIB_DIRS})

#opencv
file(GLOB OPENCV_DIRS "${PARENT_DIR}/opencv*")
list(GET OPENCV_DIRS 0 OPENCV_DIRS)  # Get the first match
set(OpenCV_DIR "${OPENCV_DIRS}/build")
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
  file(GLOB HDF5_DIRS "${PARENT_DIR}/hdf5*")
  list(GET HDF5_DIRS 0 HDF5_DIRS)  # Get the first match
  set(HDF5_USE_STATIC_LIBRARIES ON)
  set(HDF5_INCLUDE_DIRS "${HDF5_DIRS}/src")
  set(HDF5_C_INCLUDE_DIR "${HDF5_DIRS}/build/src")
  if(WIN32)
    set(HDF5_hdf5_LIBRARY_DEBUG "${HDF5_DIRS}/build/bin/Debug/libhdf5_D.lib")
    set(HDF5_hdf5_LIBRARY_RELEASE "${HDF5_DIRS}/build/bin/Release/libhdf5.lib")
  else()
    set(HDF5_hdf5_LIBRARY_DEBUG "${HDF5_DIRS}/build/bin/libhdf5.a")
    set(HDF5_hdf5_LIBRARY_RELEASE "${HDF5_DIRS}/build/bin/libhdf5.a")
  endif()
  find_package(HDF5 REQUIRED)
  if (HDF5_FOUND)
    message(STATUS "HDF5 include directories: ${HDF5_INCLUDE_DIRS}")
    message(STATUS "HDF5 libraries: ${HDF5_LIBRARIES}")
  endif()
include_directories(${HDF5_INCLUDE_DIRS}
  "${HDF5_DIRS}/src/H5FDsubfiling"
  ${HDF5_C_INCLUDE_DIR}
  )
endif()

# JNI includes
if (WIN32 OR APPLE)
  find_package(Java REQUIRED)
  find_package(JNI REQUIRED)
  include_directories(${JNI_INCLUDE_DIRS})
else()
  set(JAVA_HOME /usr/lib/jvm/jdk-19)
  set(JAVA_AWT_LIBRARY ${JAVA_HOME}/lib/libjawt.so)
  set(JAVA_JVM_LIBRARY ${JAVA_HOME}/lib/server/libjvm.so)
  set(JAVA_INCLUDE_PATH ${JAVA_HOME}/include)
  set(JAVA_INCLUDE_PATH2 ${JAVA_HOME}/include/linux)
  set(JAVA_AWT_INCLUDE_PATH ${JAVA_HOME}/include)
  find_package(Java REQUIRED)
  find_package(JNI REQUIRED)
  include_directories(${JNI_INCLUDE_DIRS})
  include_directories(${JAVA_INCLUDE_PATH})
  include_directories(${JAVA_INCLUDE_PATH2})
endif()

# FluoRender
include_directories(${FluoRender_SOURCE_DIR}/FluoRender)
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender)

# ffmpeg
file(GLOB FFMPEG_DIRS "${PARENT_DIR}/FFmpeg*")
list(GET FFMPEG_DIRS 0 FFMPEG_DIRS)  # Get the first match
set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIRS}")
#add_definitions(-D__STDC_CONSTANT_MACROS)
find_library(AVCODEC_LIBRARY avcodec HINTS "${FFMPEG_DIRS}/libavcodec")
find_library(AVFORMAT_LIBRARY avformat HINTS "${FFMPEG_DIRS}/libavformat")
find_library(AVUTIL_LIBRARY avutil HINTS "${FFMPEG_DIRS}/libavutil")
find_library(SWSCALE_LIBRARY swscale HINTS "${FFMPEG_DIRS}/libswscale")
find_library(SWRESAMPLE_LIBRARY swresample HINTS "${FFMPEG_DIRS}/libswresample")
include_directories(${FFMPEG_INCLUDE_DIR})
set(FFMPEG_LIBRARIES
    ${AVFORMAT_LIBRARY}
    ${AVCODEC_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
    ${SWRESAMPLE_LIBRARY})

# freetype
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/freetype/include)
if(WIN32)
  file(GLOB FREETYPE_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/freetype/lib/*.lib)
else()
  find_package(Freetype REQUIRED)
  include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

# HoloPlay
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/include)
if(WIN32)
	if(${ARCHITECTURE} MATCHES 32)
		file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/Win32/*.lib)
    elseif(${ARCHITECTURE} MATCHES 64)
		file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/Win64/*.lib)
    endif()
elseif(APPLE)
	file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/macos/libHoloPlayCore.dylib)
else()
	file(GLOB HoloPlay_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/linux/*.so)
endif()

#nikon
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/nd2/include)
if(WIN32)
  add_compile_options(/wd4099)
  file(GLOB ND2_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/nd2/lib/*.lib)
else()
  file(GLOB ND2_LIBRARIES ${FluoRender_SOURCE_DIR}/FluoRender/nd2/lib/*.a)
endif()

# OpenCL
find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})

# OpenVR
file(GLOB OPENVR_DIRS "${PARENT_DIR}/openvr*")
list(GET OPENVR_DIRS 0 OPENVR_DIRS)  # Get the first match
include_directories(${OPENVR_DIRS}/headers)
if(WIN32)
    file(GLOB OpenVR_LIBRARIES ${OPENVR_DIRS}/lib/win64/openvr_api.lib)
elseif(APPLE)
    file(GLOB OpenVR_LIBRARIES ${OPENVR_DIRS}/bin/osx64/libopenvr_api.dylib)
else()
    file(GLOB OpenVR_LIBRARIES ${OPENVR_DIRS}/lib/linux64/libopenvr_api.so)
endif()

# OpenXR
file(GLOB OPENXR_DIRS "${PARENT_DIR}/OpenXR-SDK*")
list(GET OPENXR_DIRS 0 OPENXR_DIRS)  # Get the first match
include_directories(${OPENXR_DIRS}/include)
if(WIN32)
    #d3d11 dependencies
    include_directories(${CMAKE_BINARY_DIR}/packages/Microsoft.Holographic.Remoting.OpenXr.2.9.4/build/native/include)
    file(GLOB OpenXR_RELEASE_LIBRARIES ${OPENXR_DIRS}/build/src/loader/Release/*.lib)
    file(GLOB OpenXR_DEBUG_LIBRARIES ${OPENXR_DIRS}/build/src/loader/Debug/*.lib)
    set(OpenXR_LIBRARIES $<$<CONFIG:Release>:${OpenXR_RELEASE_LIBRARIES} d3d11.lib dxgi.lib> $<$<CONFIG:Debug>:${OpenXR_DEBUG_LIBRARIES} d3d11.lib dxgi.lib>)
    add_definitions(-DXR_USE_PLATFORM_WIN32)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
    add_definitions(-DXR_USE_GRAPHICS_API_D3D11)
    add_definitions(-DXR_EXTENSION_PROTOTYPES)

    # nuget
	# Convert to absolute path
	get_filename_component(ABS_NUGET_EXECUTABLE "${FluoRender_SOURCE_DIR}/NuGet/nuget.exe" ABSOLUTE)
	set(NUGET_EXECUTABLE ${ABS_NUGET_EXECUTABLE})
	message(STATUS "NuGet executable path: ${NUGET_EXECUTABLE}")
	
    # Find the NuGet executable
    find_program(NUGET_COMMAND NAMES nuget PATHS "${FluoRender_SOURCE_DIR}/NuGet")

    # Ensure the NuGet executable was found
    if(NOT NUGET_COMMAND)
        message(FATAL_ERROR "NuGet executable not found. Please specify the correct path.")
    endif()

    # Specify the NuGet package and version
    set(NUGET_PACKAGE Microsoft.Holographic.Remoting.OpenXr)
    set(NUGET_VERSION 2.9.4)

    # Download and install the NuGet package
    execute_process(
        COMMAND ${NUGET_COMMAND} install ${NUGET_PACKAGE} -Version ${NUGET_VERSION} -OutputDirectory ${CMAKE_BINARY_DIR}/packages
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Set the path to the installed package
    set(NUGET_PACKAGE_DIR ${CMAKE_BINARY_DIR}/packages/${NUGET_PACKAGE}.${NUGET_VERSION})

    # Include directories
    include_directories(${NUGET_PACKAGE_DIR}/build/native/include)

    # Link libraries
    link_directories(${NUGET_PACKAGE_DIR}/build/native/lib)
elseif(APPLE)
    set(OpenXR_LIBRARIES ${OPENXR_DIRS}/build/src/loader/libopenxr_loader.dylib)
    add_definitions(-DXR_USE_PLATFORM_MACOS)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
else()
    set(OpenXR_LIBRARIES ${OPENXR_DIRS}/build/src/loader/libopenxr_loader.so)
    add_definitions(-DXR_USE_PLATFORM_XLIB)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
endif()

# pole
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/pole)
file(GLOB pole_srcs FluoRender/pole/pole.cpp)
file(GLOB pole_hdrs FluoRender/pole/pole.h)
add_library(POLE_OBJ OBJECT ${pole_srcs} ${pole_hdrs})

# teem
file(GLOB TEEM_DIRS "${PARENT_DIR}/teem*")
list(GET TEEM_DIRS 0 TEEM_DIRS)  # Get the first match
set(Teem_DIR "${TEEM_DIRS}/build")
find_package(Teem REQUIRED)
include(${Teem_USE_FILE})
include_directories(${Teem_INCLUDE_DIRS})
set(Nrrd_DIR "${TEEM_DIRS}/src/nrrd")
include_directories(${Nrrd_DIR})
# teem required definitions
add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)
add_definitions(-DTEEM_ZLIB=1)
add_definitions(-DTEEM_32BIT=0)

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
if (APPLE)
add_library(LOOKINGGLASS_OBJ OBJECT
  ${lgr_src} ${lgr_hdr} ${HoloPlay_LIBRARIES})
else()
add_library(LOOKINGGLASS_OBJ OBJECT
  ${lgr_src} ${lgr_hdr})
endif()

# OpenXR
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/OpenXR)
file(GLOB oxr_src FluoRender/FluoRender/OpenXR/*.cpp)
file(GLOB oxr_hdr FluoRender/FluoRender/OpenXR/*.h)
add_library(OPENXR_OBJ OBJECT
  ${oxr_src} ${oxr_hdr})

# Progress
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Progress)
file(GLOB prg_src FluoRender/FluoRender/Progress/*.cpp)
file(GLOB prg_hdr FluoRender/FluoRender/Progress/*.h)
add_library(PROGRESS_OBJ OBJECT
  ${prg_src} ${prg_hdr})

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

# Trainer
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Trainer)
file(GLOB trainer_src FluoRender/FluoRender/Trainer/*.cpp)
file(GLOB trainer_hdr FluoRender/FluoRender/Trainer/*.h)
add_library(TRAINER_OBJ OBJECT
  ${trainer_src} ${trainer_hdr})

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

# other sources
file(GLOB src FluoRender/FluoRender/*.cpp)
file(GLOB hdr FluoRender/FluoRender/*.h)

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
  set_target_properties(ANIMATORS_OBJ PROPERTIES MACOSX_DEPOLYMENT_TARGET "11.5")
  set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
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
    $<TARGET_OBJECTS:OPENXR_OBJ>
    $<TARGET_OBJECTS:PROGRESS_OBJ>
    $<TARGET_OBJECTS:PROJECT_OBJ>
    $<TARGET_OBJECTS:PYTHON_OBJ>
    $<TARGET_OBJECTS:SCENE_OBJ>
    $<TARGET_OBJECTS:SCRIPT_OBJ>
    $<TARGET_OBJECTS:SELECTION_OBJ>
    $<TARGET_OBJECTS:TIMER_OBJ>
    $<TARGET_OBJECTS:TRACKING_OBJ>
    $<TARGET_OBJECTS:TRAINER_OBJ>
    $<TARGET_OBJECTS:TYPE_OBJ>
    $<TARGET_OBJECTS:UTILITY_OBJ>
    $<TARGET_OBJECTS:VIDEO_OBJ>
    $<TARGET_OBJECTS:POLE_OBJ>
    )
  set_target_properties(FluoRender PROPERTIES MACOSX_DEPLOYMENT_TARGET "11.5")
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
    $<TARGET_OBJECTS:OPENXR_OBJ>
    $<TARGET_OBJECTS:PROGRESS_OBJ>
    $<TARGET_OBJECTS:PROJECT_OBJ>
    $<TARGET_OBJECTS:PYTHON_OBJ>
    $<TARGET_OBJECTS:SCENE_OBJ>
    $<TARGET_OBJECTS:SCRIPT_OBJ>
    $<TARGET_OBJECTS:SELECTION_OBJ>
    $<TARGET_OBJECTS:TIMER_OBJ>
    $<TARGET_OBJECTS:TRACKING_OBJ>
    $<TARGET_OBJECTS:TRAINER_OBJ>
    $<TARGET_OBJECTS:TYPE_OBJ>
    $<TARGET_OBJECTS:UTILITY_OBJ>
    $<TARGET_OBJECTS:VIDEO_OBJ>
    $<TARGET_OBJECTS:POLE_OBJ>
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
      if (USE_MP_COMPILE)
        add_definitions(/MP)
      endif()
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
      $<TARGET_OBJECTS:OPENXR_OBJ>
      $<TARGET_OBJECTS:PROGRESS_OBJ>
      $<TARGET_OBJECTS:PROJECT_OBJ>
      $<TARGET_OBJECTS:PYTHON_OBJ>
      $<TARGET_OBJECTS:SCENE_OBJ>
      $<TARGET_OBJECTS:SCRIPT_OBJ>
      $<TARGET_OBJECTS:SELECTION_OBJ>
      $<TARGET_OBJECTS:TIMER_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:TRAINER_OBJ>
      $<TARGET_OBJECTS:TYPE_OBJ>
      $<TARGET_OBJECTS:UTILITY_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
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
      $<TARGET_OBJECTS:OPENXR_OBJ>
      $<TARGET_OBJECTS:PROGRESS_OBJ>
      $<TARGET_OBJECTS:PROJECT_OBJ>
      $<TARGET_OBJECTS:PYTHON_OBJ>
      $<TARGET_OBJECTS:SCENE_OBJ>
      $<TARGET_OBJECTS:SCRIPT_OBJ>
      $<TARGET_OBJECTS:SELECTION_OBJ>
      $<TARGET_OBJECTS:TIMER_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:TRAINER_OBJ>
      $<TARGET_OBJECTS:TYPE_OBJ>
      $<TARGET_OBJECTS:UTILITY_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:WACUTILS_OBJ>
    )
    endif()
    if(BUILD_TESTER)
        add_executable(Tester
          ${tester_src} ${tester_hdr}
          $<TARGET_OBJECTS:CONTROL_OBJ>
          $<TARGET_OBJECTS:DATABASE_OBJ>
          $<TARGET_OBJECTS:FILE_OBJ>
          $<TARGET_OBJECTS:FLOBJECT_OBJ>
          $<TARGET_OBJECTS:IMAGE_OBJ>
          $<TARGET_OBJECTS:TIMER_OBJ>
          $<TARGET_OBJECTS:TRAINER_OBJ>
          $<TARGET_OBJECTS:TYPE_OBJ>
          $<TARGET_OBJECTS:UTILITY_OBJ>
          $<TARGET_OBJECTS:VIDEO_OBJ>
          $<TARGET_OBJECTS:POLE_OBJ>
          $<TARGET_OBJECTS:WACUTILS_OBJ>
          $<TARGET_OBJECTS:XINPUT_OBJ>
        )
    endif()
  endif()
endif()

# architecture specific rules
#if(${ARCHITECTURE} MATCHES 64)
#  if(APPLE)
#    set(ARCH_FLAGS "-m64 -arch x86_64")
#  endif()
#  set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}")
#  set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
#  set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
#endif()

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
    ${Boost_LIBRARIES}
    dlib::dlib
    ${FFMPEG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${GLEW_STATIC_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${HoloPlay_LIBRARIES}
    ${ND2_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARIES}
    ${OpenVR_LIBRARIES}
    ${OpenXR_LIBRARIES}
    ${PNG_LIBRARIES}
    ${Python3_LIBRARIES}
    teem
    ${TIFF_LIBRARIES}
    secur32.lib
    ${wxWidgets_LIBRARIES}
    XInput.lib
    ${ZLIB_LIBRARIES}
  )
  if(BUILD_TESTER)
      target_link_libraries(Tester
        ${Boost_LIBRARIES}
        dlib::dlib
        ${FFMPEG_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${GLEW_STATIC_LIBRARIES}
        ${HDF5_LIBRARIES}
        ${HoloPlay_LIBRARIES}
        ${ND2_LIBRARIES}
        ${OpenCL_LIBRARIES}
        ${OpenCV_LIBS}
        ${OPENGL_LIBRARIES}
        ${OpenVR_LIBRARIES}
        ${OpenXR_LIBRARIES}
        ${PNG_LIBRARIES}
        ${Python3_LIBRARIES}
        teem
        ${TIFF_LIBRARIES}
        secur32.lib
        ${wxWidgets_LIBRARIES}
        XInput.lib
        ${ZLIB_LIBRARIES}
      )
  endif()
elseif(APPLE)
  target_link_libraries(FluoRender
    dlib::dlib
    ${OPENGL_LIBRARIES}
    ${GLEW_STATIC_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    teem
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ND2_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${OpenCV_LIBS}
    ${HoloPlay_LIBRARIES}
    ${OpenVR_LIBRARIES}
    ${OpenXR_LIBRARIES}
  )
else()
  target_link_libraries(FluoRender
    stdc++fs
    dlib::dlib
    ${OPENGL_LIBRARIES}
    ${GLEW_STATIC_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
    ${OpenCL_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    teem
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ND2_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${OpenCV_LIBS}
    ${HoloPlay_LIBRARIES}
    ${OpenVR_LIBRARIES}
    ${OpenXR_LIBRARIES}
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
	"${OPENVR_DIRS}/bin/win64/openvr_api.dll"
    "$<TARGET_FILE_DIR:FluoRender>")
elseif(APPLE)
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${OPENVR_DIRS}/bin/osx64/libopenvr_api.dylib"
    "$<TARGET_FILE_DIR:FluoRender>")
else()
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${OPENVR_DIRS}/bin/linux64/libopenvr_api.so"
    "$<TARGET_FILE_DIR:FluoRender>")
endif()

# openxr
if(WIN32)
set(OPENXR_DLL_DEBUG "${OPENXR_DIRS}/build/src/loader/Debug/openxr_loaderd.dll")
set(OPENXR_DLL_RELEASE "${OPENXR_DIRS}/build/src/loader/Release/openxr_loader.dll")
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<IF:$<CONFIG:Debug>,${OPENXR_DLL_DEBUG},${OPENXR_DLL_RELEASE}>
    "$<TARGET_FILE_DIR:FluoRender>")
elseif(APPLE)
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${OPENXR_DIRS}/build/src/loader/libopenxr_loader.dylib"
    "$<TARGET_FILE_DIR:FluoRender>")
else()
add_custom_command(TARGET FluoRender POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"${OPENXR_DIRS}/build/src/loader/libopenxr_loader.so"
    "$<TARGET_FILE_DIR:FluoRender>")
endif()

# holoplay
if(WIN32)
    add_custom_command(TARGET FluoRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/Win64/HoloPlayCore.dll"
        "$<TARGET_FILE_DIR:FluoRender>"
    )
elseif(APPLE)
    add_custom_command(TARGET FluoRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${HoloPlay_LIBRARIES}"
        "$<TARGET_FILE_DIR:FluoRender>"
        )
    add_custom_command(TARGET FluoRender POST_BUILD COMMAND
        ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/"
        $<TARGET_FILE:FluoRender>)
else()
    add_custom_command(TARGET FluoRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${FluoRender_SOURCE_DIR}/FluoRender/HoloPlayCore/dylib/linux/libHoloPlayCore.so"
        "$<TARGET_FILE_DIR:FluoRender>")
endif()

# hololens
if(WIN32)
    file(GLOB HOLOLENS_FILES "${FluoRender_SOURCE_DIR}/FluoRender/Hololens/*.*")
    add_custom_command(TARGET FluoRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${HOLOLENS_FILES}
        "$<TARGET_FILE_DIR:FluoRender>"
    )
endif()