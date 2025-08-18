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

cmake_minimum_required(VERSION 3.10)

# option to build with multithreading
SET(USE_MP_COMPILE "0" CACHE STRING "Number of parallel jobs for building")
# Convert the string to a number
math(EXPR MP_THREADS "${USE_MP_COMPILE}")

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
# option to build tester
  SET(BUILD_TESTER OFF CACHE BOOL "Enable building the tester")
endif()

# for MSVC builds
if(WIN32)
  if(MSVC)
    SET(MSVC_INCREMENTAL_DEFAULT OFF)
  endif()
endif()

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3" CACHE STRING "Minimum OS X deployment version")

  # Detect native architecture
  execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE NATIVE_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Map uname output to CMake architecture
  if(NATIVE_ARCH STREQUAL "arm64")
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Target architecture" FORCE)
  elseif(NATIVE_ARCH STREQUAL "x86_64")
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Target architecture" FORCE)
  else()
    message(WARNING "Unknown architecture: ${NATIVE_ARCH}. Defaulting to x86_64.")
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Target architecture" FORCE)
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

#if (WIN32 OR APPLE)
#else()
#  set(CMAKE_C_COMPILER "gcc-12")
#  set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
#endif()

#if(UNIX)
#  set(RELEASE_FLAGS ${RELEASE_FLAGS} -O3 -march=corei7-avx)
#  set(DEBUG_FLAGS ${DEBUG_FLAGS} -O0 -march=corei7-avx -ggdb)
#endif()

add_definitions(-DFLUORENDER_TITLE="FluoRender")

add_definitions(-DVERSION_MAJOR=2)						# Be sure to update the Versions and Date for each release!!!!
add_definitions(-DVERSION_MINOR=33)						# 
add_definitions(-DVERSION_MAJOR_TAG="2")				# 
add_definitions(-DVERSION_MINOR_TAG="33")				# 
add_definitions(-DVERSION_COPYRIGHT="July 2025")	    # Up to here!

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
# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# wxWidgets
file(GLOB WXWIDGETS_DIRS "${PARENT_DIR}/wxWidgets*")
list(GET WXWIDGETS_DIRS 0 WXWIDGETS_DIR)  # Get the first match
message(STATUS "wxWidgets directory: ${WXWIDGETS_DIR}")
if(WIN32)
    # Windows-specific settings
    set(wxWidgets_ROOT_DIR ${WXWIDGETS_DIR})
    set(wxWidgets_LIB_DIR ${WXWIDGETS_DIR}/lib/vc_x64_lib)
    set(wxWidgets_INCLUDE_DIRS ${WXWIDGETS_DIR}/include)
    if(${CMAKE_BUILD_TYPE} MATCHES "Release")
      set(wxWidgets_CONFIGURATION mswu)
    elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
      set(wxWidgets_CONFIGURATION mswud)
    endif()
else()
    set(wxWidgets_ROOT_DIR ${WXWIDGETS_DIR})
    set(wxWidgets_CONFIG_EXECUTABLE ${WXWIDGETS_DIR}/mybuild/wx-config)
    set(wxWidgets_LIB_DIR ${WXWIDGETS_DIR}/mybuild/lib)
endif()
#set(wxWidgets_USE_REL_AND_DBG ON)
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
# libtiff
if(WIN32)
    set(Tiff_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(TIFF_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(PNG_PNG_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/png)
    set(JPEG_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/jpeg)
    set(ZLIB_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/zlib)
    set(TIFF_LIBRARY_RELEASE ${wxWidgets_LIB_DIR}/wxtiff.lib)
    set(TIFF_LIBRARY_DEBUG ${wxWidgets_LIB_DIR}/wxtiffd.lib)
    set(PNG_LIBRARY_RELEASE ${wxWidgets_LIB_DIR}/wxpng.lib)
    set(PNG_LIBRARY_DEBUG ${wxWidgets_LIB_DIR}/wxpngd.lib)
    set(ZLIB_LIBRARY_RELEASE ${wxWidgets_LIB_DIR}/wxzlib.lib)
    set(ZLIB_LIBRARY_DEBUG ${wxWidgets_LIB_DIR}/wxzlibd.lib)
elseif(APPLE)
    set(TIFF_INCLUDE_DIR "${wxWidgets_ROOT_DIR}/src/tiff/libtiff")
    set(PNG_PNG_INCLUDE_DIR "${wxWidgets_ROOT_DIR}/src/png")
    set(JPEG_INCLUDE_DIR "${wxWidgets_ROOT_DIR}/src/jpeg")
    set(ZLIB_INCLUDE_DIR "${wxWidgets_ROOT_DIR}/src/zlib")
    # TIFF
    file(GLOB TIFF_LIBRARIES "${wxWidgets_LIB_DIR}/libwxtiff-*.a")
    list(GET TIFF_LIBRARIES 0 TIFF_LIBRARY)
    set(TIFF_LIBRARY_RELEASE "${TIFF_LIBRARY}")
    set(TIFF_LIBRARY_DEBUG "${TIFF_LIBRARY}")
    # PNG
    file(GLOB PNG_LIBRARIES "${wxWidgets_LIB_DIR}/libwxpng-*.a")
    list(GET PNG_LIBRARIES 0 PNG_LIBRARY)
    set(PNG_LIBRARY_RELEASE "${PNG_LIBRARY}")
    set(PNG_LIBRARY_DEBUG "${PNG_LIBRARY}")
    # ZLIB
    file(GLOB ZLIB_LIBRARIES "${wxWidgets_LIB_DIR}/libwxzlib-*.a")
    list(GET ZLIB_LIBRARIES 0 ZLIB_LIBRARY)
    set(ZLIB_LIBRARY_RELEASE "${ZLIB_LIBRARY}")
    set(ZLIB_LIBRARY_DEBUG "${ZLIB_LIBRARY}")
endif()
find_package(TIFF REQUIRED)
find_package(PNG REQUIRED)
#find_package(JPEG REQUIRED)
#find_package(ZLIB REQUIRED)
if(WIN32)
    set(Tiff_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
    set(ZLIB_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/zlib)
    set(TIFF_INCLUDE_DIR ${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
endif()
message(STATUS "wxWidgets_LIB_DIR: ${wxWidgets_LIB_DIR}")
message(STATUS "wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")

# Boost
file(GLOB BOOST_DIRS "${PARENT_DIR}/boost*")
list(GET BOOST_DIRS 0 BOOST_DIRS)  # Get the first match
set(Boost_INCLUDE_DIR "${BOOST_DIRS}")
# only find static libs:
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system filesystem locale graph REQUIRED)

# bzip2
if (NOT WIN32)
file(GLOB BZIP2_DIRS "${PARENT_DIR}/bzip2*")
list(GET BZIP2_DIRS 0 BZIP2_DIRS)  # Get the first match
set(BZIP2_ROOT "${BZIP2_DIRS}")
find_path(BZIP2_INCLUDE_DIR bzlib.h PATHS "${BZIP2_ROOT}/include")
find_library(BZIP2_LIBRARY bz2 PATHS "${BZIP2_ROOT}/lib")
endif()

# dlib
if(WIN32)
    file(GLOB BLAS_DIRS "${PARENT_DIR}/OpenBLAS*")
    list(GET BLAS_DIRS 0 BLAS_DIRS)  # Get the first match
    set(OpenBLAS_DIR "${BLAS_DIRS}/build")
    set(OpenBLAS_LIB_DIR "${BLAS_DIRS}/build/lib")
    set(BLAS_openblas_LIBRARY "${BLAS_DIRS}/build/lib/RELEASE/openblas.lib")
    set(CUDNN_ROOT_DIR $ENV{CUDNN_PATH})
    message (STATUS "CUDNN_ROOT_DIR=${CUDNN_ROOT_DIR}")
    set(cudnn "${CUDNN_ROOT_DIR}/lib/12.5/x64/cudnn.lib")
    message (STATUS cudnn "cudnn=${cudnn}")
    set(cudnn_include "${CUDNN_ROOT_DIR}/include/12.5/")
    message (STATUS cudnn_include "cudnn_include=${cudnn_include}")
endif()
file(GLOB DLIB_DIRS "${PARENT_DIR}/dlib*")
list(GET DLIB_DIRS 0 DLIB_DIRS)  # Get the first match
add_subdirectory(${DLIB_DIRS} dlib_build)
set(DLIB_USE_BLAS ON)
set(DLIB_USE_CUDA ON)

#opencv
file(GLOB OPENCV_DIRS "${PARENT_DIR}/opencv*")
list(GET OPENCV_DIRS 0 OPENCV_DIRS)  # Get the first match
set(OpenCV_DIR "${OPENCV_DIRS}/build")
find_package(OpenCV REQUIRED)
if (OPENCV_FOUND)
  message (STATUS "OpenCV_INCLUDE_DIRS=${OpenCV_INCLUDE_DIRS}")
endif()

#python
#set(Python_ADDITIONAL_VERSIONS 3.10 3.9 3.8 3.7)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
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
  set(HDF5_INCLUDE_PATHS
    ${HDF5_INCLUDE_DIRS}
    "${HDF5_DIRS}/src/H5FDsubfiling"
    ${HDF5_C_INCLUDE_DIR}
)
endif()

# JNI includes
if (WIN32 OR APPLE)
  find_package(Java REQUIRED)
  find_package(JNI REQUIRED)
else()
  set(JAVA_HOME /usr/lib/jvm/jdk-19)
  set(JAVA_AWT_LIBRARY ${JAVA_HOME}/lib/libjawt.so)
  set(JAVA_JVM_LIBRARY ${JAVA_HOME}/lib/server/libjvm.so)
  set(JAVA_INCLUDE_PATH ${JAVA_HOME}/include)
  set(JAVA_INCLUDE_PATH2 ${JAVA_HOME}/include/linux)
  set(JAVA_AWT_INCLUDE_PATH ${JAVA_HOME}/include)
  find_package(Java REQUIRED)
  find_package(JNI REQUIRED)
endif()

# ffmpeg
file(GLOB FFMPEG_DIRS "${PARENT_DIR}/FFmpeg*")
list(GET FFMPEG_DIRS 0 FFMPEG_DIRS)  # Get the first match
set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIRS}")
find_library(AVCODEC_LIBRARY avcodec HINTS "${FFMPEG_DIRS}/libavcodec")
find_library(AVFORMAT_LIBRARY avformat HINTS "${FFMPEG_DIRS}/libavformat")
find_library(AVUTIL_LIBRARY avutil HINTS "${FFMPEG_DIRS}/libavutil")
find_library(SWSCALE_LIBRARY swscale HINTS "${FFMPEG_DIRS}/libswscale")
find_library(SWRESAMPLE_LIBRARY swresample HINTS "${FFMPEG_DIRS}/libswresample")
set(FFMPEG_LIBRARIES
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${SWSCALE_LIBRARY}
    ${SWRESAMPLE_LIBRARY})
if(WIN32)
set(X26x_RELEASE_LIBRARIES
    ${FFMPEG_DIRS}/depends/lib/libx264.lib
    ${FFMPEG_DIRS}/depends/lib/libx265.lib)
set(X26x_DEBUG_LIBRARIES
    ${FFMPEG_DIRS}/depends/lib/libx264.lib
    ${FFMPEG_DIRS}/depends/lib/libx265d.lib)
list(APPEND FFMPEG_LIBRARIES
    $<$<CONFIG:Debug>:${X26x_DEBUG_LIBRARIES}>
    $<$<CONFIG:Release>:${X26x_RELEASE_LIBRARIES}>
    mfplat.lib
    mfuuid.lib
    strmiids.lib
    bcrypt.lib
)
elseif(APPLE)
    find_library(X264_LIBRARY x264 HINTS "${FFMPEG_DIRS}/depends/lib")
    find_library(X265_LIBRARY x265 HINTS "${FFMPEG_DIRS}/depends/lib")
    list(APPEND FFMPEG_LIBRARIES
        ${X264_LIBRARY}
        ${X265_LIBRARY})
endif()

# freetype
if(WIN32)
    file(GLOB FREETYPE_DIRS "${PARENT_DIR}/freetype*")
    list(GET FREETYPE_DIRS 0 FREETYPE_DIRS)  # Get the first match
    set(FREETYPE_INCLUDE_DIRS "${FREETYPE_DIRS}/include")
    set(FREETYPE_LIBRARY_RELEASE "${FREETYPE_DIRS}/build/Release/freetype.lib")
    set(FREETYPE_LIBRARY_DEBUG "${FREETYPE_DIRS}/build/Debug/freetyped.lib")
endif()
find_package(Freetype REQUIRED)

# glm
file(GLOB GLM_DIRS "${PARENT_DIR}/glm*")
list(GET GLM_DIRS 0 GLM_DIRS)  # Get the first match
add_definitions(-DGLM_FORCE_RADIANS=1)

# BridgeRuntime
set(BRIDGE_RUNTIME_PATHS "${FluoRender_SOURCE_DIR}/FluoRender/BridgeRuntime")

# nd2
if(WIN32)
  add_compile_options(/wd4099)
  set(ND2_PATHS "${FluoRender_SOURCE_DIR}/FluoRender/nd2/Win64/include")
  # Define the base path to your ND2 SDK libs
  set(ND2_LIB_DIR "${FluoRender_SOURCE_DIR}/FluoRender/nd2/Win64/lib")
  
  # Create a variable that resolves to the correct .lib file based on configuration
  set(ND2_LIBRARIES
      $<$<CONFIG:Debug>:
          ${ND2_LIB_DIR}/nd2readsdk-staticd.lib;
          ${ND2_LIB_DIR}/limfile-staticd.lib
      >
      $<$<NOT:$<CONFIG:Debug>>:
          ${ND2_LIB_DIR}/nd2readsdk-static.lib;
          ${ND2_LIB_DIR}/limfile-static.lib
      >
  )
elseif(APPLE)
  # Detect architecture: x86_64 or arm64
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(NIKON_ARCH "arm64")
  else()
    set(NIKON_ARCH "x86_64")
  endif()

  # Construct paths based on detected architecture
  set(ND2_LIB_DIR "${FluoRender_SOURCE_DIR}/FluoRender/nd2/MacOS/${NIKON_ARCH}/lib")
  set(ND2_INCLUDE_DIR "${FluoRender_SOURCE_DIR}/FluoRender/nd2/macOS/${NIKON_ARCH}/include")

  file(GLOB ND2_LIBRARIES "${ND2_LIB_DIR}/*.a")
  set(ND2_PATHS "${ND2_INCLUDE_DIR}")
endif()

# OpenCL
if(APPLE)
    find_library(OpenCL_LIBRARY OpenCL)
    set(OpenCL_INCLUDE_DIRS "/System/Library/Frameworks/OpenCL.framework/Headers")
    set(OpenCL_LIBRARIES ${OpenCL_LIBRARY})
else()
    find_package(OpenCL REQUIRED)
endif()
add_definitions(-DCL_TARGET_OPENCL_VERSION=300)

# OpenVR
file(GLOB OPENVR_DIRS "${PARENT_DIR}/openvr*")
list(GET OPENVR_DIRS 0 OPENVR_DIRS)  # Get the first match
if(WIN32)
    file(GLOB OpenVR_LIBRARIES ${OPENVR_DIRS}/lib/win64/openvr_api.lib)
elseif(APPLE)
    file(GLOB OpenVR_LIBRARIES ${OPENVR_DIRS}/bin/osx64/libopenvr_api.a)
else()
    file(GLOB OpenVR_LIBRARIES ${OPENVR_DIRS}/lib/linux64/libopenvr_api.so)
endif()
set(OPENVR_PATHS "${OPENVR_DIRS}/headers")

# OpenXR
file(GLOB OPENXR_DIRS "${PARENT_DIR}/OpenXR-SDK*")
list(GET OPENXR_DIRS 0 OPENXR_DIRS)  # Get the first match
set(OPENXR_PATHS "${OPENXR_DIRS}/include")
if(WIN32)
    #d3d11 dependencies
    set(OPENXR_PATHS "${OPENXR_PATHS};${CMAKE_BINARY_DIR}/packages/Microsoft.Holographic.Remoting.OpenXr.2.9.4/build/native/include")
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
elseif(APPLE)
    set(OpenXR_LIBRARIES ${OPENXR_DIRS}/build/src/loader/libopenxr_loader.dylib)
    add_definitions(-DXR_USE_PLATFORM_MACOS)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
else()
    set(OpenXR_LIBRARIES ${OPENXR_DIRS}/build/src/loader/libopenxr_loader.so)
    add_definitions(-DXR_USE_PLATFORM_XLIB)
    add_definitions(-DXR_USE_GRAPHICS_API_OPENGL)
endif()

# teem
file(GLOB TEEM_DIRS "${PARENT_DIR}/teem*")
list(GET TEEM_DIRS 0 TEEM_DIRS)  # Get the first match

if(WIN32)
    set(Teem_DIR "${TEEM_DIRS}/build")
else()
    set(Teem_DIR "${TEEM_DIRS}/install/lib/cmake/Teem")
endif()

find_package(Teem REQUIRED)

if(DEFINED Teem_USE_FILE AND EXISTS "${Teem_USE_FILE}")
    include(${Teem_USE_FILE})
endif()

set(TEEM_PATHS ${Teem_INCLUDE_DIRS}
    "${TEEM_DIRS}/src/nrrd")

# teem required definitions
add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)
add_definitions(-DTEEM_ZLIB=1)
add_definitions(-DTEEM_32BIT=0)


# Tester
if(BUILD_TESTER)
  file(GLOB tester_hdr FluoRender/Tester/*.h)
  file(GLOB tester_src FluoRender/Tester/*.cpp)
endif()

# Wacom
if(WIN32)
    file(GLOB wac_src FluoRender/FluoRender/WacomUtils/*.cpp)
    file(GLOB wac_hdr FluoRender/FluoRender/WacomUtils/*.h)
    add_library(WACOMUTILS_LIB STATIC
      ${wac_src} ${wac_hdr})
    set(WACOM_PATHS "${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/WacomUtils/Include"
      "${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/WacomUtils")
    target_include_directories(WACOMUTILS_LIB PRIVATE
      ${WACOM_PATHS})
endif()

# XInput
if(WIN32 AND USE_XINPUT)
	file(GLOB xip_src FluoRender/FluoRender/XInput/*.cpp)
	file(GLOB xip_hdr FluoRender/FluoRender/XInput/*.h)
	add_library(XINPUT_LIB STATIC
		${xip_src} ${xip_hdr})
	set(XINPUT_PATHS "${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/XInput")
    target_include_directories(XINPUT_LIB PRIVATE
      ${XINPUT_PATHS})
    target_link_libraries(XINPUT_LIB PRIVATE
        XInput.lib)
endif()

# Animator
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Animator)
file(GLOB ani_src FluoRender/FluoRender/Animator/*.cpp)
file(GLOB ani_hdr FluoRender/FluoRender/Animator/*.h)
add_library(ANIMATORS_LIB STATIC
  ${ani_src} ${ani_hdr})

# Calculate
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Calculate)
file(GLOB calc_src FluoRender/FluoRender/Calculate/*.cpp)
file(GLOB calc_hdr FluoRender/FluoRender/Calculate/*.h)
add_library(CALCULATE_LIB STATIC
  ${calc_src} ${calc_hdr})

# Cluster
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Cluster)
file(GLOB clstr_src FluoRender/FluoRender/Cluster/*.cpp)
file(GLOB clstr_hdr FluoRender/FluoRender/Cluster/*.h)
add_library(CLUSTER_LIB STATIC
  ${clstr_src} ${clstr_hdr})

# Component
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Component)
file(GLOB cmp_src FluoRender/FluoRender/Component/*.cpp)
file(GLOB cmp_hdr FluoRender/FluoRender/Component/*.h)
add_library(COMPONENT_LIB STATIC
  ${cmp_src} ${cmp_hdr})

# Config
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Config)
file(GLOB config_src FluoRender/FluoRender/Config/*.cpp)
file(GLOB config_hdr FluoRender/FluoRender/Config/*.h)
add_library(CONFIG_LIB STATIC
  ${config_src} ${config_hdr})

# Control
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Control)
file(GLOB ctrl_src FluoRender/FluoRender/Control/*.cpp)
file(GLOB ctrl_hdr FluoRender/FluoRender/Control/*.h)
add_library(CONTROL_LIB STATIC
  ${ctrl_src} ${ctrl_hdr})

# Converter
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Converter)
file(GLOB cvt_src FluoRender/FluoRender/Converter/*.cpp)
file(GLOB cvt_hdr FluoRender/FluoRender/Converter/*.h)
add_library(CONVERTER_LIB STATIC
  ${cvt_src} ${cvt_hdr})

# Database
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Database)
file(GLOB dbs_src FluoRender/FluoRender/Database/*.cpp)
file(GLOB dbs_hdr FluoRender/FluoRender/Database/*.h)
add_library(DATABASE_LIB STATIC
  ${dbs_src} ${dbs_hdr})

# Distance
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Distance)
file(GLOB dist_src FluoRender/FluoRender/Distance/*.cpp)
file(GLOB dist_hdr FluoRender/FluoRender/Distance/*.h)
add_library(DISTANCE_LIB STATIC
  ${dist_src} ${dist_hdr})

# File
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/File)
file(GLOB file_src_c FluoRender/FluoRender/File/*.c)
file(GLOB file_src_cpp FluoRender/FluoRender/File/*.cpp)
file(GLOB file_hdr FluoRender/FluoRender/File/*.h)
set(file_src ${file_src_c} ${file_src_cpp})
add_library(FILE_LIB STATIC
  ${file_src} ${file_hdr})

# Filter
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Filter)
file(GLOB fltr_src FluoRender/FluoRender/Filter/*.cpp)
file(GLOB fltr_hdr FluoRender/FluoRender/Filter/*.h)
add_library(FILTER_LIB STATIC
  ${fltr_src} ${fltr_hdr})

# Flobject
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Flobject)
file(GLOB obj_src FluoRender/FluoRender/Flobject/*.cpp)
file(GLOB obj_hdr FluoRender/FluoRender/Flobject/*.hpp)
add_library(FLOBJECT_LIB STATIC
  ${obj_src} ${obj_hdr})

# Flrender
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Flrender)
file(GLOB flrd_src FluoRender/FluoRender/Flrender/*.cpp)
file(GLOB flrd_hdr FluoRender/FluoRender/Flrender/*.h)
add_library(FLRENDER_LIB STATIC
  ${flrd_src} ${flrd_hdr})

# Flui
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Flui)
file(GLOB flui_src FluoRender/FluoRender/Flui/*.cpp)
file(GLOB flui_hdr FluoRender/FluoRender/Flui/*.h)
add_library(FLUI_LIB STATIC
  ${flui_src} ${flui_hdr})

# Format
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Format)
file(GLOB fmt_src FluoRender/FluoRender/Format/*.cpp)
file(GLOB fmt_hdr FluoRender/FluoRender/Format/*.h)
add_library(FORMAT_LIB STATIC
  ${fmt_src} ${fmt_hdr} ${fmt_rsc})

# Global
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Global)
file(GLOB glb_src FluoRender/FluoRender/Global/*.cpp)
file(GLOB glb_hdr FluoRender/FluoRender/Global/*.h)
add_library(GLOBAL_LIB STATIC
  ${glb_src} ${glb_hdr})

# Image
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Image)
file(GLOB img_hdr FluoRender/FluoRender/Image/*.h)
file(GLOB img_src FluoRender/FluoRender/Image/*.cpp)
file(GLOB img_rsc FluoRender/FluoRender/Image/*.xpm)
add_library(IMAGE_LIB STATIC
  ${img_hdr} ${img_src} ${img_rsc})

# LookingGlass
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/LookingGlass)
file(GLOB lgr_src FluoRender/FluoRender/LookingGlass/*.cpp)
file(GLOB lgr_hdr FluoRender/FluoRender/LookingGlass/*.h)
add_library(LOOKINGGLASS_LIB STATIC
  ${lgr_src} ${lgr_hdr})

# Main
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender)
file(GLOB src FluoRender/FluoRender/*.cpp)
file(GLOB hdr FluoRender/FluoRender/*.h)
add_library(MAIN_LIB STATIC
  ${src} ${hdr})

# OpenXR
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/OpenXR)
file(GLOB oxr_src FluoRender/FluoRender/OpenXR/*.cpp)
file(GLOB oxr_hdr FluoRender/FluoRender/OpenXR/*.h)
add_library(OPENXR_LIB STATIC
  ${oxr_src} ${oxr_hdr})

# Progress
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Progress)
file(GLOB prg_src FluoRender/FluoRender/Progress/*.cpp)
file(GLOB prg_hdr FluoRender/FluoRender/Progress/*.h)
add_library(PROGRESS_LIB STATIC
  ${prg_src} ${prg_hdr})

# Project
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Project)
file(GLOB prj_src FluoRender/FluoRender/Project/*.cpp)
file(GLOB prj_hdr FluoRender/FluoRender/Project/*.h)
add_library(PROJECT_LIB STATIC
  ${prj_src} ${prj_hdr})

# Python
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Python)
file(GLOB pyth_hdr FluoRender/FluoRender/Python/*.h)
file(GLOB pyth_src FluoRender/FluoRender/Python/*.cpp)
add_library(PYTHON_LIB STATIC
  ${pyth_hdr} ${pyth_src})

# Scene
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Scene)
file(GLOB scn_src FluoRender/FluoRender/Scene/*.cpp)
file(GLOB scn_hdr FluoRender/FluoRender/Scene/*.h)
add_library(SCENE_LIB STATIC
  ${scn_src} ${scn_hdr})

# Script
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Script)
file(GLOB scr_src FluoRender/FluoRender/Script/*.cpp)
file(GLOB scr_hdr FluoRender/FluoRender/Script/*.h)
add_library(SCRIPT_LIB STATIC
  ${scr_src} ${scr_hdr})

# Selection
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Selection)
file(GLOB sel_src FluoRender/FluoRender/Selection/*.cpp)
file(GLOB sel_hdr FluoRender/FluoRender/Selection/*.h)
add_library(SELECTION_LIB STATIC
  ${sel_src} ${sel_hdr})

# Timer
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Timer)
file(GLOB timer_src FluoRender/FluoRender/Timer/*.cpp)
file(GLOB timer_hdr FluoRender/FluoRender/Timer/*.hpp)
add_library(TIMER_LIB STATIC
  ${timer_src} ${timer_hdr})

# Tracking
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Tracking)
file(GLOB trk_src FluoRender/FluoRender/Tracking/*.cpp)
file(GLOB trk_hdr FluoRender/FluoRender/Tracking/*.h)
add_library(TRACKING_LIB STATIC
  ${trk_src} ${trk_hdr})

# Trainer
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Trainer)
file(GLOB trainer_src FluoRender/FluoRender/Trainer/*.cpp)
file(GLOB trainer_hdr FluoRender/FluoRender/Trainer/*.h)
add_library(TRAINER_LIB STATIC
  ${trainer_src} ${trainer_hdr})

# Type
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Type)
file(GLOB typ_src FluoRender/FluoRender/Type/*.cpp)
file(GLOB typ_hdr FluoRender/FluoRender/Type/*.h)
add_library(TYPE_LIB STATIC
  ${typ_src} ${typ_hdr})

# Utility
include_directories(${FluoRender_SOURCE_DIR}/FluoRender/FluoRender/Utility)
file(GLOB utl_src FluoRender/FluoRender/Utility/*.cpp)
file(GLOB utl_hdr FluoRender/FluoRender/Utility/*.h)
file(GLOB utl_rsc FluoRender/FluoRender/Utility/*.natvis)
add_library(UTILITY_LIB STATIC
  ${utl_src} ${utl_hdr} ${utl_rsc})

# include and link
# Animator
target_include_directories(ANIMATORS_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLM_DIRS}
  ${TEEM_PATHS}
  ${TIFF_INCLUDE_DIR})
if (APPLE)
    target_link_libraries(ANIMATORS_LIB PRIVATE
      ${BZIP2_LIBRARY})
endif()
target_link_libraries(ANIMATORS_LIB PRIVATE
  ${TIFF_LIBRARIES})

# Calculate
target_include_directories(CALCULATE_LIB PRIVATE
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${TEEM_PATHS})

# Cluster
target_include_directories(CLUSTER_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLM_DIRS}
  ${TEEM_PATHS})
target_link_libraries(CLUSTER_LIB PRIVATE
  ${Boost_LIBRARIES})

# Component
target_include_directories(COMPONENT_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${TEEM_PATHS})
target_link_libraries(COMPONENT_LIB PRIVATE
  ${Boost_LIBRARIES})

# Config
target_include_directories(CONFIG_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${TEEM_PATHS})
target_link_libraries(CONFIG_LIB PRIVATE
  ${GLEW_STATIC_LIBRARIES})

# Control

# Converter
target_include_directories(CONVERTER_LIB PRIVATE
  ${GLM_DIRS}
  ${TEEM_PATHS})
target_link_libraries(CONVERTER_LIB PRIVATE
  teem)

# Database
target_include_directories(DATABASE_LIB PRIVATE
  ${DLIB_DIRS}
  ${Boost_INCLUDE_DIR})

# Distance
target_include_directories(DISTANCE_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${TEEM_PATHS})
target_link_libraries(DISTANCE_LIB PRIVATE
  teem
  ${GLEW_STATIC_LIBRARIES}
  ${OpenCV_LIBS})

# File
target_include_directories(FILE_LIB PRIVATE
  ${Boost_INCLUDE_DIR})

# Filter
target_include_directories(FILTER_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${TEEM_PATHS})
target_link_libraries(FILTER_LIB PRIVATE
  ${Boost_LIBRARIES})

# Flobject
target_include_directories(FLOBJECT_LIB PRIVATE
  ${Boost_INCLUDE_DIR})

# Flrender
target_include_directories(FLRENDER_LIB PRIVATE
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${TEEM_PATHS})
target_link_libraries(FLRENDER_LIB PRIVATE
  teem
  ${FREETYPE_LIBRARIES}
  ${GLEW_STATIC_LIBRARIES}
  ${OpenCL_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

# Flui
target_include_directories(FLUI_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${Python3_INCLUDE_DIRS}
  ${TEEM_PATHS})
if (WIN32)
  target_include_directories(FLUI_LIB PRIVATE
    ${WACOM_PATHS})
endif()
if (WIN32 OR APPLE)
  target_include_directories(FLUI_LIB PRIVATE
    ${JNI_INCLUDE_DIRS})
else()
  target_include_directories(FLUI_LIB PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${JAVA_INCLUDE_PATH}
    ${JAVA_INCLUDE_PATH2})
endif()
target_link_libraries(FLUI_LIB PRIVATE
  ${GLEW_STATIC_LIBRARIES})
if(WIN32)
    target_link_libraries(FLUI_LIB PRIVATE
      WACOMUTILS_LIB)
endif()
if (WIN32 AND USE_XINPUT)
    target_link_libraries(FLUI_LIB PRIVATE
      XINPUT_LIB)
endif()

# Format
target_include_directories(FORMAT_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${FFMPEG_INCLUDE_DIR}
  ${GLM_DIRS}
  ${JPEG_INCLUDE_DIR}
  ${ND2_PATHS}
  ${PNG_PNG_INCLUDE_DIR}
  ${TEEM_PATHS}
  ${TIFF_INCLUDE_DIR})
if (WIN32 OR APPLE)
  target_include_directories(FORMAT_LIB PRIVATE
    ${JNI_INCLUDE_DIRS})
else()
  target_include_directories(FORMAT_LIB PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${JAVA_INCLUDE_PATH}
    ${JAVA_INCLUDE_PATH2})
endif()
if (APPLE)
    target_link_libraries(FORMAT_LIB PRIVATE
      ${BZIP2_LIBRARY})
endif()
target_link_libraries(FORMAT_LIB PRIVATE
  teem
  ${Boost_LIBRARIES}
  ${FFMPEG_LIBRARIES}
  ${ND2_LIBRARIES}
  ${PNG_LIBRARIES}
  ${TIFF_LIBRARIES}
)

# Global
target_include_directories(GLOBAL_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OPENXR_PATHS}
  ${Python3_INCLUDE_DIRS}
  ${TEEM_PATHS})
if (WIN32 OR APPLE)
  target_include_directories(GLOBAL_LIB PRIVATE
    ${JNI_INCLUDE_DIRS})
else()
  target_include_directories(GLOBAL_LIB PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${JAVA_INCLUDE_PATH}
    ${JAVA_INCLUDE_PATH2})
endif()

# Image

# LookingGlass
target_include_directories(LOOKINGGLASS_LIB PRIVATE
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${BRIDGE_RUNTIME_PATHS})
target_link_libraries(LOOKINGGLASS_LIB PRIVATE
  ${GLEW_STATIC_LIBRARIES})

# Main
target_include_directories(MAIN_LIB PRIVATE
  ${GLM_DIRS}
  ${TEEM_PATHS})
target_link_libraries(MAIN_LIB PRIVATE
  ${wxWidgets_LIBRARIES})

# OpenXR
target_include_directories(OPENXR_LIB PRIVATE
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OPENVR_PATHS}
  ${OPENXR_PATHS})
target_link_libraries(OPENXR_LIB PRIVATE
  ${GLEW_STATIC_LIBRARIES}
  ${OpenVR_LIBRARIES}
  ${OpenXR_LIBRARIES})

# Progress
target_include_directories(PROGRESS_LIB PRIVATE
  ${GLM_DIRS}
  ${TEEM_PATHS})

# Project
target_include_directories(PROJECT_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLM_DIRS}
  ${TEEM_PATHS})

# Python
target_include_directories(PYTHON_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${Python3_INCLUDE_DIRS})
if (WIN32 OR APPLE)
  target_include_directories(PYTHON_LIB PRIVATE
    ${HDF5_INCLUDE_PATHS})
endif()
target_link_libraries(PYTHON_LIB PRIVATE
  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES})

# Scene
target_include_directories(SCENE_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${TEEM_PATHS}
  ${TIFF_INCLUDE_DIR})
if (WIN32)
  target_include_directories(SCENE_LIB PRIVATE
    ${WACOM_PATHS})
endif()
if (WIN32 OR APPLE)
  target_include_directories(SCENE_LIB PRIVATE
    ${JNI_INCLUDE_DIRS})
else()
  target_include_directories(SCENE_LIB PRIVATE
    ${JNI_INCLUDE_DIRS}
    ${JAVA_INCLUDE_PATH}
    ${JAVA_INCLUDE_PATH2})
endif()
if (APPLE)
    target_link_libraries(SCENE_LIB PRIVATE
      ${BZIP2_LIBRARY})
endif()
target_link_libraries(SCENE_LIB PRIVATE
  ${TIFF_LIBRARIES}
  ${GLEW_STATIC_LIBRARIES})

# Script
target_include_directories(SCRIPT_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLM_DIRS}
  ${Python3_INCLUDE_DIRS}
  ${TEEM_PATHS}
  ${TIFF_INCLUDE_DIR})
if (APPLE)
    target_link_libraries(SCRIPT_LIB PRIVATE
      ${BZIP2_LIBRARY})
endif()
target_link_libraries(SCRIPT_LIB PRIVATE
  ${TIFF_LIBRARIES}
  ${Python3_LIBRARIES})

# Selection
target_include_directories(SELECTION_LIB PRIVATE
  ${GLEW_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${TEEM_PATHS})
target_link_libraries(SELECTION_LIB PRIVATE
  ${GLEW_STATIC_LIBRARIES})

# Timer
target_include_directories(TIMER_LIB PRIVATE
  ${Boost_INCLUDE_DIR})

# Tracking
target_include_directories(TRACKING_LIB PRIVATE
  ${Boost_INCLUDE_DIR}
  ${GLM_DIRS}
  ${OpenCL_INCLUDE_DIRS}
  ${TEEM_PATHS})
target_link_libraries(TRACKING_LIB PRIVATE
  ${Boost_LIBRARIES})

# Trainer
target_link_libraries(TRAINER_LIB PRIVATE
  ${Python3_LIBRARIES}
  dlib::dlib)

# Type
target_include_directories(TYPE_LIB PRIVATE
  ${GLEW_INCLUDE_DIR})
target_link_libraries(TYPE_LIB PRIVATE
  ${GLEW_STATIC_LIBRARIES})

# Utility
target_link_libraries(UTILITY_LIB PRIVATE
  ${Boost_LIBRARIES})

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

# Platform-specific rules
if(APPLE)
  # Set macOS deployment target and standard flags
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3")
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -DWX_PRECOMP")
  #set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development")
  set(CMAKE_XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep --timestamp")
  #set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "")
  #set(CMAKE_XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES")

  # Add macOS frameworks
  set(MACOS_FRAMEWORKS
    "-framework OpenCL"
    "-framework CoreFoundation"
    "-framework CoreVideo"
    "-framework VideoDecodeAcceleration"
    "-framework VideoToolbox"
    "-framework Security"
    "-framework CoreMedia"
  )

  # Add executable with bundle
  add_executable(FluoRender MACOSX_BUNDLE ${src} ${hdr} ${rsc} ${icns})

  # Set RPATH for bundled app
  set_target_properties(FluoRender PROPERTIES
    INSTALL_RPATH "@executable_path"
    BUILD_WITH_INSTALL_RPATH TRUE
    MACOSX_BUNDLE TRUE
  )

  # Add OpenGL header defines if not set
  if(NOT OPEN_GL_HEADER_LOC)
    add_definitions(-DOPEN_GL_HEADER_LOC="<OpenGL/gl.h>")
  endif()
  if(NOT OPEN_GLU_HEADER_LOC)
    add_definitions(-DOPEN_GLU_HEADER_LOC="<OpenGL/glu.h>")
  endif()

  add_definitions(-D_DARWIN)

  # Link macOS frameworks
  target_link_libraries(FluoRender PRIVATE ${MACOS_FRAMEWORKS})

elseif(WIN32)
  # Windows-specific definitions
  add_definitions(-DWIN32 -D_WIN32 -DNOMINMAX)

  if(MSVC)
    if(MP_THREADS GREATER 1)
      add_definitions(/MP)
    endif()
    add_definitions(
      -D_CRT_NONSTDC_NO_DEPRECATE
      -D_CRT_SECURE_NO_DEPRECATE
      -Dinline=__inline
      "/wd4273"
      -D_XKEYCHECK_H
    )
    set(CMAKE_CXX_FLAGS "/EHsc")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG /INCREMENTAL /OPT:NOREF")
  endif()

  add_executable(FluoRender WIN32 ${src} ${hdr} ${rsc} ${icns})

  if(BUILD_TESTER)
    add_executable(Tester ${tester_src} ${tester_hdr})
    target_include_directories(Tester PRIVATE ${FluoRender_SOURCE_DIR}/FluoRender/Tester)
  endif()

elseif(UNIX)
  # Linux or other UNIX-like systems
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  add_executable(FluoRender ${src} ${hdr} ${rsc} ${icns})
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread -ldl")
endif()


target_compile_options(FluoRender PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_FLAGS}>")
target_compile_options(FluoRender PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_FLAGS}>")

target_include_directories(FluoRender PRIVATE
  ${GLM_DIRS}
  ${TEEM_PATHS})
# link the libraries
if(WIN32)
  target_link_libraries(FluoRender PRIVATE
    secur32.lib
  )
elseif(UNIX AND NOT APPLE)
  target_link_libraries(FluoRender PRIVATE
    stdc++fs
  )
endif()

target_link_libraries(FluoRender PRIVATE
    MAIN_LIB
    ANIMATORS_LIB
    CALCULATE_LIB
    CLUSTER_LIB
    COMPONENT_LIB
    CONFIG_LIB
    CONTROL_LIB
    CONVERTER_LIB
    DATABASE_LIB
    DISTANCE_LIB
    FILE_LIB
    FILTER_LIB
    FLOBJECT_LIB
    FLRENDER_LIB
    FLUI_LIB
    FORMAT_LIB
    GLOBAL_LIB
    IMAGE_LIB
    LOOKINGGLASS_LIB
    OPENXR_LIB
    PROGRESS_LIB
    PROJECT_LIB
    PYTHON_LIB
    SCENE_LIB
    SCRIPT_LIB
    SELECTION_LIB
    TIMER_LIB
    TRACKING_LIB
    TRAINER_LIB
    TYPE_LIB
    UTILITY_LIB
)
if(BUILD_TESTER)
    target_link_libraries(Tester PRIVATE
      ${Boost_LIBRARIES}
      dlib::dlib
      FLOBJECT_LIB
      TYPE_LIB
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
  COMMAND ${CMAKE_COMMAND} -E copy
  "${FluoRender_SOURCE_DIR}/Settings/fluorender.xml"
  "$<TARGET_FILE_DIR:FluoRender>")

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

# hololens
if(WIN32)
    file(GLOB HOLOLENS_FILES "${FluoRender_SOURCE_DIR}/FluoRender/Hololens/*.*")
    add_custom_command(TARGET FluoRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${HOLOLENS_FILES}
        "$<TARGET_FILE_DIR:FluoRender>"
    )
endif()

if(APPLE)
# Path to your final app binary
set(TARGET_BINARY "$<TARGET_FILE:FluoRender>")

# Replace hardcoded Python framework with generic version
add_custom_command(
    TARGET FluoRender
    POST_BUILD
    COMMAND install_name_tool -change
      /Library/Frameworks/Python.framework/Versions/3.13/Python
      /Library/Frameworks/Python.framework/Python
      ${TARGET_BINARY}
    COMMENT "Rewriting Python framework path before signing"
)
endif()
