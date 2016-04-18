#/*
#For more information, please see: http://software.sci.utah.edu
#
#The MIT License
#
#Copyright (c) 2014 Scientific Computing and Imaging Institute,
#University of Utah.
#
#
#Permission is hereby granted, free of charge, to any person obtaining a
#copy of this software and associated documentation files (the "Software"),
#to deal in the Software without restriction, including without limitation
#the rights to use, copy, modify, merge, publish, distribute, sublicense,
#and/or sell copies of the Software, and to permit persons to whom the
#Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included
#in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#DEALINGS IN THE SOFTWARE.
#*/

#This is an CMake configuration file for FluoRender

cmake_minimum_required ( VERSION 2.8.8 )

#for MSVC builds
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(MSVC)
		SET(MSVC_INCREMENTAL_DEFAULT OFF)
	endif()
endif()

project ( FluoRender )

IF((COMMAND cmake_policy) AND NOT (CMAKE_MAJOR_VERSION LESS 3))
    CMAKE_POLICY(SET CMP0040 NEW)
    CMAKE_POLICY(SET CMP0043 NEW)
ENDIF()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X
   set(ARCHITECTURE 64)
else()
   # Windows
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

add_definitions(-DFLUORENDER_TITLE="FluoRender")

add_definitions(-DVERSION_MAJOR=2)                   #Be sure to update the Versions and Date for each release!!!!
add_definitions(-DVERSION_MINOR=19)					 #
add_definitions(-DVERSION_MAJOR_TAG="2")             #
add_definitions(-DVERSION_MINOR_TAG="19")            #
add_definitions(-DVERSION_COPYRIGHT="Apr 2016")   # Up to here!

#static compile
add_definitions(-DSTATIC_COMPILE)

#teem required definitions
add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

#output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#wxWidgets
set(wxWidgets_USE_REL_AND_DBG ON)
if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	set(wxWidgets_CONFIGURATION mswu)
elseif(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(wxWidgets_CONFIGURATION mswud)
endif()
find_package(wxWidgets COMPONENTS core base aui html xml adv gl stc scintilla REQUIRED)
include(${wxWidgets_USE_FILE})
set(wxWidgets_USE_STATIC ON)
if (APPLE)
   find_package(TIFF REQUIRED)
   include_directories(${TIFF_INCLUDE_DIR})
else()
   include_directories(${wxWidgets_ROOT_DIR}/src/tiff/libtiff)
endif()
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
#solution for wxWidgets linking "ambiguous" errors
add_definitions(-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_)
#fix the library names for OSX if needed. 
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  foreach(arg ${wxWidgets_LIBRARIES})
    set(wxlib_out "${wxlib_out} ${arg}")
  endforeach()
  string(STRIP ${wxlib_out} wxWidgets_LIBRARIES)
  string(REGEX REPLACE "libwx_osx_cocoau_scintilla" "libwxscintilla" 
                   wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES})
  find_package(Freetype REQUIRED)
  include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

#OpenGL
find_package(OpenGL REQUIRED)

#Boost
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system chrono filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

#teem
# add zlib
find_package(ZLIB)
if (ZLIB_FOUND)
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

#glew
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
file(GLOB gl_srcs fluorender/glew/*.c)
file(GLOB gl_hdrs1 fluorender/glew/*.h)
file(GLOB gl_hdrs2 fluorender/glew/GL/*.h)
add_library(GLEW_OBJ OBJECT
      ${gl_srcs} ${gl_hdrs1} ${gl_hdrs2})

#pole
include_directories(${FluoRender_SOURCE_DIR}/fluorender/pole)
file(GLOB pole_srcs fluorender/pole/pole.cpp)
file(GLOB pole_hdrs fluorender/pole/pole.h)
add_library(POLE_OBJ OBJECT ${pole_srcs} ${pole_hdrs})

#ffmpeg
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  include_directories(${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/include)
  file(GLOB ffmpeg_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/lib/Win64/*.a ${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/lib/Win64/*.lib)
  add_definitions(-D__STDC_CONSTANT_MACROS)
else()
  include_directories(${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/include)
  file(GLOB ffmpeg_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/ffmpeg/lib/OSX/*.a)
endif()

#FluoRender
include_directories(${FluoRender_SOURCE_DIR}/fluorender)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender)

#Formats
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Formats)
file(GLOB fmt_src fluorender/FluoRender/Formats/*.cpp)
file(GLOB fmt_hdr fluorender/FluoRender/Formats/*.h)
add_library(FORMATS_OBJ OBJECT
      ${fmt_src} ${fmt_hdr})

#Video
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Video)
file(GLOB vid_src fluorender/FluoRender/Video/*.cpp)
file(GLOB vid_hdr fluorender/FluoRender/Video/*.h)
add_library(VIDEO_OBJ OBJECT
      ${vid_src} ${vid_hdr})

#Tracking
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Tracking)
file(GLOB trk_src fluorender/FluoRender/Tracking/*.cpp)
file(GLOB trk_hdr fluorender/FluoRender/Tracking/*.h)
add_library(TRACKING_OBJ OBJECT
      ${trk_src} ${trk_hdr})

#Components
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Components)
file(GLOB cmp_src fluorender/FluoRender/Components/*.cpp)
file(GLOB cmp_hdr fluorender/FluoRender/Components/*.h)
add_library(COMPONENTS_OBJ OBJECT
      ${cmp_src} ${cmp_hdr})

#Animators
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Animators)
file(GLOB ani_src fluorender/FluoRender/Animator/*.cpp)
file(GLOB ani_hdr fluorender/FluoRender/Animator/*.h)
add_library(ANIMATORS_OBJ OBJECT
      ${ani_src} ${ani_hdr})

#NV
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/NV)
file(GLOB nv_src fluorender/FluoRender/NV/*.cpp)
file(GLOB nv_hdr fluorender/FluoRender/NV/*.h)
add_library(NV_OBJ OBJECT
      ${nv_src} ${nv_hdr})

#FLIVR
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/FLIVR)
file(GLOB fli_src fluorender/FluoRender/FLIVR/*.cpp)
file(GLOB fli_hdr fluorender/FluoRender/FLIVR/*.h)
add_library(FLIVR_OBJ OBJECT
      ${fli_src} ${fli_hdr})

#Converters
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Converters)
file(GLOB cvt_src fluorender/FluoRender/Converters/*.cpp)
file(GLOB cvt_hdr fluorender/FluoRender/Converters/*.h)
add_library(CONVERTERS_OBJ OBJECT
      ${cvt_src} ${cvt_hdr})

#images
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/img)
file(GLOB pngs_hdr fluorender/FluoRender/img/*.h)
file(GLOB pngs_src fluorender/FluoRender/img/*.cpp)
add_library(IMAGES_OBJ OBJECT  ${pngs_hdr} ${pngs_src})

#OpenCL
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  include_directories(${FluoRender_SOURCE_DIR}/fluorender/OpenCL/include)
  if(${ARCHITECTURE} MATCHES 64)
    file(GLOB OPENCL_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/OpenCL/lib/x86_64/*.lib)
  elseif(${ARCHITECTURE} MATCHES 32)
    file(GLOB OPENCL_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/OpenCL/lib/x86/*.lib)
  endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  include(${CMAKE_SOURCE_DIR}/FindOpenCL.cmake)
  include_directories( ${OPENCL_INCLUDE_DIRS})
endif()

#freetype
include_directories(${FluoRender_SOURCE_DIR}/fluorender/freetype/include)
if (WIN32)
    file(GLOB FREETYPE_LIBRARIES ${FluoRender_SOURCE_DIR}/fluorender/freetype/lib/*.lib)
else()
    find_package(Freetype REQUIRED)
    include_directories(${FREETYPE_INCLUDE_DIRS})
endif()

#other sources
file(GLOB src fluorender/FluoRender/*.cpp)
file(GLOB hdr fluorender/FluoRender/*.h)
file(GLOB rsc fluorender/FluoRender/img/*.xpm)

#SynthBB
include_directories(${FluoRender_SOURCE_DIR}/fluorender/SynthBB)
file(GLOB synthbb_hdr fluorender/SynthBB/*.h)
file(GLOB synthbb_src fluorender/SynthBB/*.cpp)

#MaskInt
include_directories(${FluoRender_SOURCE_DIR}/fluorender/MaskInt)
file(GLOB maskint_hdr fluorender/MaskInt/*.h)
file(GLOB maskint_src fluorender/MaskInt/*.cpp)

#platform specific rules
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
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
   #icon file for Apple, bundle properties
   set(ICON_FILE_PATH "FluoRender_icon")
   SET( MACOSX_BUNDLE_ICON_FILE ${ICON_FILE_PATH} )
   SET_SOURCE_FILES_PROPERTIES(${ICON_FILE_PATH} PROPERTIES 
      MACOSX_BUNDLE_LOCATION Resources)
   add_executable(FluoRender MACOSX_BUNDLE
      ${src} ${hdr} ${rsc}
      $<TARGET_OBJECTS:IMAGES_OBJ>
      $<TARGET_OBJECTS:CONVERTERS_OBJ>
      $<TARGET_OBJECTS:FLIVR_OBJ>
      $<TARGET_OBJECTS:ANIMATORS_OBJ>
      $<TARGET_OBJECTS:NV_OBJ>
      $<TARGET_OBJECTS:FORMATS_OBJ>
      $<TARGET_OBJECTS:VIDEO_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:COMPONENTS_OBJ>
      $<TARGET_OBJECTS:GLEW_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>)
   add_executable(SynthBB
      ${synthbb_src} ${synthbb_hdr}
      $<TARGET_OBJECTS:FLIVR_OBJ>
      $<TARGET_OBJECTS:FORMATS_OBJ>
      $<TARGET_OBJECTS:TRACKING_OBJ>
      $<TARGET_OBJECTS:COMPONENTS_OBJ>
      $<TARGET_OBJECTS:GLEW_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>)
   add_executable(MaskInt
      ${maskint_src} ${maskint_hdr}
      $<TARGET_OBJECTS:FORMATS_OBJ>
      $<TARGET_OBJECTS:POLE_OBJ>
      $<TARGET_OBJECTS:TEEM_OBJ>)

elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   #Windows
   if(NOT OPEN_GL_HEADER_LOC)
      add_definitions(-DOPEN_GL_HEADER_LOC=<GL/gl.h>)
   endif()
   if(NOT OPEN_GLU_HEADER_LOC)
      add_definitions(-DOPEN_GLU_HEADER_LOC=<GL/glu.h>)
   endif()
   # windows
   if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      add_definitions(-DWIN32)
      add_definitions(-D_WIN32)
	  if(MSVC)
		  add_definitions(-DSEQAN_C++11_STANDARD=ON)
		  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
		  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
		  add_definitions(-Dinline=__inline)
		  add_definitions("/wd4273")
		  add_definitions(-D_XKEYCHECK_H)
		  set(CFLAGS "")
		  set(CXXFLAGS "/EHsc")
		  #make sure the reference option is turned off and not incremental build linking
		  STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG}) 
		  STRING(REPLACE "INCREMENTAL:NO:NO" "INCREMENTAL:NO" replacementFlags1 ${replacementFlags}) 
		  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO /OPT:NOREF ${replacementFlags1}" )

		  STRING(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags2 ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
		  STRING(REPLACE "INCREMENTAL:NO:NO" "INCREMENTAL:NO" replacementFlags3 ${replacementFlags2})
		  SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO /OPT:NOREF ${replacementFlags3}" )

		  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /OPT:NOREF" )
	  endif()
      include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/WacUtils)
      include_directories(${FluoRender_SOURCE_DIR}/fluorender/Wacom/Include)
      file(GLOB wac_src fluorender/FluoRender/WacUtils/*.cpp)
      file(GLOB wac_hdr fluorender/FluoRender/WacUtils/*.h)
	  add_library(WACUTILS_OBJ OBJECT
			     ${wac_src} ${wac_hdr})
      add_executable(FluoRender WIN32
          	  ${src} ${hdr} ${rsc}
		  $<TARGET_OBJECTS:IMAGES_OBJ>
		  $<TARGET_OBJECTS:CONVERTERS_OBJ>
		  $<TARGET_OBJECTS:FLIVR_OBJ>
		  $<TARGET_OBJECTS:ANIMATORS_OBJ>
		  $<TARGET_OBJECTS:NV_OBJ>
		  $<TARGET_OBJECTS:FORMATS_OBJ>
		  $<TARGET_OBJECTS:VIDEO_OBJ>
		  $<TARGET_OBJECTS:TRACKING_OBJ>
		  $<TARGET_OBJECTS:COMPONENTS_OBJ>
		  $<TARGET_OBJECTS:GLEW_OBJ>
		  $<TARGET_OBJECTS:POLE_OBJ>
		  $<TARGET_OBJECTS:TEEM_OBJ>
		  $<TARGET_OBJECTS:WACUTILS_OBJ>)
      add_executable(SynthBB
          	  ${synthbb_src} ${synthbb_hdr}
		  $<TARGET_OBJECTS:FLIVR_OBJ>
		  $<TARGET_OBJECTS:FORMATS_OBJ>
		  $<TARGET_OBJECTS:TRACKING_OBJ>
		  $<TARGET_OBJECTS:COMPONENTS_OBJ>
		  $<TARGET_OBJECTS:GLEW_OBJ>
		  $<TARGET_OBJECTS:POLE_OBJ>
		  $<TARGET_OBJECTS:TEEM_OBJ>)
      add_executable(MaskInt
         ${maskint_src} ${maskint_hdr}
         $<TARGET_OBJECTS:FORMATS_OBJ>
         $<TARGET_OBJECTS:POLE_OBJ>
         $<TARGET_OBJECTS:TEEM_OBJ>)
   endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif()

#architecture specific rules
if(${ARCHITECTURE} MATCHES 64)
   if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      set(ARCH_FLAGS "-m64 -arch x86_64")
   endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   add_definitions(-DTEEM_32BIT=0)
   set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}")
   set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
   set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
else()
   if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      set(ARCH_FLAGS "-m32 -arch i386")
   endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   add_definitions(-DTEEM_32BIT=1)
   set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}" )
   set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
   set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/lib -L/usr/lib ${CMAKE_EXE_LINKER_FLAGS} -lbz2 -framework OpenCL -framework CoreFoundation -framework CoreVideo -framework VideoDecodeAcceleration -framework VideoToolbox -framework Security -framework CoreMedia")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.9 -DWX_PRECOMP -std=c++11 -stdlib=libc++")
   #make sure png is static linked
   string(REGEX REPLACE "-lpng" "/usr/local/lib/libpng.a" 
			wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES})
   string(REGEX REPLACE "-lpng" "/usr/local/lib/libpng.a" 
			CMAKE_MAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
   string(REGEX REPLACE "/usr/local/lib/libfreetype.dylib" "/usr/local/lib/libfreetype.a;/usr/local/lib/libpng.a" 
     FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES})
   message(STATUS "${FREETYPE_LIBRARIES}")
endif()

#link the libraries
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
target_link_libraries(FluoRender
   ${ffmpeg_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${Boost_LIBRARIES}
   ${OPENCL_LIBRARIES}
   ${FREETYPE_LIBRARIES}
   ${wxWidgets_LIBRARIES})
elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
target_link_libraries(FluoRender
   secur32.lib
   ${ffmpeg_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${Boost_LIBRARIES}
   ${OPENCL_LIBRARIES}
   ${FREETYPE_LIBRARIES}
   ${wxWidgets_LIBRARIES})
endif()

target_link_libraries(SynthBB
   ${Boost_LIBRARIES}
   ${OPENCL_LIBRARIES}
   ${wxWidgets_LIBRARIES})
target_link_libraries(MaskInt
   ${Boost_LIBRARIES}
   ${wxWidgets_LIBRARIES})

#build OpenCL examples copies.

#copy openCL examples to the binary directory
add_custom_command(TARGET FluoRender PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/CL_code"
  "$<TARGET_FILE_DIR:FluoRender>/CL_code")
add_custom_command(TARGET FluoRender PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/CL_code"
  "${CMAKE_BINARY_DIR}/CL_code")
   
#copy font dir to the binary directory
add_custom_command(TARGET FluoRender PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Fonts"
  "$<TARGET_FILE_DIR:FluoRender>/Fonts")
add_custom_command(TARGET FluoRender PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Fonts"
  "${CMAKE_BINARY_DIR}/Fonts")
  
#copy script dir to the binary directory
add_custom_command(TARGET FluoRender PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${FluoRender_SOURCE_DIR}/Scripts"
  "$<TARGET_FILE_DIR:FluoRender>/Scripts")

#copy executables (Windows)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_custom_command(TARGET FluoRender PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${FluoRender_SOURCE_DIR}/Executables"
		"$<TARGET_FILE_DIR:FluoRender>/Executables")
	add_custom_command(TARGET FluoRender PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${FluoRender_SOURCE_DIR}/Executables"
		"${CMAKE_BINARY_DIR}/Executables")
endif()
   
#post build setting files copy
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	file(GLOB settings_files Settings/*)
	foreach (f ${settings_files})
		add_custom_command(TARGET FluoRender PRE_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy  
			${f}
			$<TARGET_FILE_DIR:FluoRender>)
	endforeach()
endif()
