#This is an initial CMake configuration file for FluoRender

cmake_minimum_required ( VERSION 2.6 )

project ( FluoRender )

set(ARCHITECTURE 64 CACHE PATH "The architecture to build -- 32 or 64")

set(CMAKE_BUILD_TYPE "Debug" CACHE PATH "Debug or Release Configuration")

if(${ARCHITECTURE} MATCHES "64")
   if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
      add_definitions(-DFLUORENDER_TITLE="FluoRender 64bit Debug")
   else()
      add_definitions(-DFLUORENDER_TITLE="FluoRender 64bit")
   endif()
else()
   if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
      add_definitions(-DFLUORENDER_TITLE="FluoRender 32bit Debug")
   else()
      add_definitions(-DFLUORENDER_TITLE="FluoRender 32bit")
   endif()
endif()

add_definitions(-DVERSION_MAJOR=2)
add_definitions(-DVERSION_MINOR=14)
add_definitions(-DVERSION_MAJOR_TAG="2")
add_definitions(-DVERSION_MINOR_TAG="14")
add_definitions(-DVERSION_COPYRIGHT="January 2014")

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base aui html xrc xml net adv gl REQUIRED)
include(${wxWidgets_USE_FILE})
set(wxWidgets_USE_STATIC ON)

find_package(OpenGL REQUIRED)

#temporary solution for wxWidgets linking "ambiguous" errors
add_definitions(-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_)

#static compile
add_definitions(-DSTATIC_COMPILE)

#teem required definitions
add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

#output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#includes
include_directories(${FluoRender_SOURCE_DIR}/fluorender)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/LibTiff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Formats)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Animators)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/NV)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/FLIVR)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Converters)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/img)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glm)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glm/detail)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glm/gtc)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glm/gtx)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glm/virtrev)

#sources
file(GLOB src fluorender/FluoRender/*.cpp)
file(GLOB hdr fluorender/FluoRender/*.h)
file(GLOB rsc fluorender/FluoRender/img/*.xpm)

file(GLOB pngs fluorender/FluoRender/img/*.h)

file(GLOB ani_src fluorender/FluoRender/Animator/*.cpp)
file(GLOB ani_hdr fluorender/FluoRender/Animator/*.h)

file(GLOB cvt_src fluorender/FluoRender/Converters/*.cpp)
file(GLOB cvt_hdr fluorender/FluoRender/Converters/*.h)

file(GLOB fli_src fluorender/FluoRender/FLIVR/*.cpp)
file(GLOB fli_hdr fluorender/FluoRender/FLIVR/*.h)

file(GLOB fmt_src fluorender/FluoRender/Formats/*.cpp)
file(GLOB fmt_hdr fluorender/FluoRender/Formats/*.h)

file(GLOB nv_src fluorender/FluoRender/NV/*.cpp)
file(GLOB nv_hdr fluorender/FluoRender/NV/*.h)

file(GLOB gl_srcs fluorender/glew/*.c)
file(GLOB gl_hdrs1 fluorender/glew/*.h)
file(GLOB gl_hdrs2 fluorender/glew/GL/*.h)

file(GLOB airsrc fluorender/teem/Air/*.c)
file(GLOB airhdr fluorender/teem/Air/*.h)
file(GLOB hestsrc fluorender/teem/Hest/*.c)
file(GLOB hesthdr fluorender/teem/Hest/*.h)
file(GLOB biffsrc fluorender/teem/Biff/*.c)
file(GLOB biffhdr fluorender/teem/Biff/*.h)
file(GLOB nrrdsrc fluorender/teem/Nrrd/*.c)
file(GLOB nrrdhdr fluorender/teem/Nrrd/*.h)

file(GLOB glm1 fluorender/glm/*.hpp)
file(GLOB glm2 fluorender/glm/*/*.hpp)


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
   set (CXX_11_FLAG "-std=c++0x")
   set(CFLAGS "-fPIC")
   set(CXXFLAGS "-fPIC -Wno-unused-private-field")
   include(BundleUtilities)
   set(ICON_FILE_PATH "FluoRender_icon")
   #add_resource("fluorender/FluoRender/img/FluoRender_icon.gif")
   SET( MACOSX_BUNDLE_ICON_FILE ${ICON_FILE_PATH} )
   SET_SOURCE_FILES_PROPERTIES(${ICON_FILE_PATH} PROPERTIES 
      MACOSX_BUNDLE_LOCATION Resources)
   add_executable(FluoRender MACOSX_BUNDLE
      ${src} ${hdr} ${rsc} ${pngs}
      ${ani_src} ${ani_hdr}
      ${cvt_src} ${cvt_hdr}
      ${fli_src} ${fli_hdr}
      ${fmt_src} ${fmt_hdr}
      ${nv_src} ${nv_hdr}
      ${gl_srcs} ${gl_hdrs1} ${gl_hdrs2}
      ${airsrc} ${hestsrc} ${nrrdsrc} ${biffsrc}
      ${airhdr} ${hesthdr} ${nrrdhdr} ${biffhdr}
      ${glm1} ${glm2}
      ${wac_src} ${wac_hdr})

elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Linux or Windows
   if(NOT OPEN_GL_HEADER_LOC)
      add_definitions(-DOPEN_GL_HEADER_LOC=<GL/gl.h>)
   endif()
   if(NOT OPEN_GLU_HEADER_LOC)
      add_definitions(-DOPEN_GLU_HEADER_LOC=<GL/glu.h>)
   endif()
   # linux
   if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      add_definitions(-D_LINUX)
      set (CXX_11_FLAG "-std=c++0x")
      set(CFLAGS "-fPIC")
      set(CXXFLAGS "-fPIC -Wno-unused-private-field")
      add_executable(FluoRender
         ${src} ${hdr} ${rsc} ${pngs}
         ${ani_src} ${ani_hdr}
         ${cvt_src} ${cvt_hdr}
         ${fli_src} ${fli_hdr}
         ${fmt_src} ${fmt_hdr}
         ${nv_src} ${nv_hdr}
         ${gl_srcs} ${gl_hdrs1} ${gl_hdrs2}
         ${airsrc} ${hestsrc} ${nrrdsrc} ${biffsrc}
         ${airhdr} ${hesthdr} ${nrrdhdr} ${biffhdr}
         ${glm1} ${glm2}
         ${wac_src} ${wac_hdr})
   endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # windows
   if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      add_definitions(-DWIN32)
      add_definitions(-D_WIN32)
      add_definitions(-DSEQAN_C++11_STANDARD=ON)
      add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
      add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
      add_definitions("/wd4273")
      set(CFLAGS "")
      set(CXXFLAGS "/EHsc")
      include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/WacUtils)
      include_directories(${FluoRender_SOURCE_DIR}/fluorender/Wacom/Include)
      file(GLOB wac_src fluorender/FluoRender/WacUtils/*.cpp)
      file(GLOB wac_hdr fluorender/FluoRender/WacUtils/*.h)
      add_executable(FluoRender WIN32
         ${src} ${hdr} ${rsc} ${pngs}
         ${ani_src} ${ani_hdr}
         ${cvt_src} ${cvt_hdr}
         ${fli_src} ${fli_hdr}
         ${fmt_src} ${fmt_hdr}
         ${nv_src} ${nv_hdr}
         ${gl_srcs} ${gl_hdrs1} ${gl_hdrs2}
         ${airsrc} ${hestsrc} ${nrrdsrc} ${biffsrc}
         ${airhdr} ${hesthdr} ${nrrdhdr} ${biffhdr}
         ${glm1} ${glm2}
         ${wac_src} ${wac_hdr})
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
   set(LDFLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
else()
   if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      set(ARCH_FLAGS "-m32 -arch i386")
   endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   add_definitions(-DTEEM_32BIT=1)
   set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}" )
   set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
   set(LDFLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
endif()

#link the libraries
target_link_libraries(FluoRender
   ${wxWidgets_LIBRARIES}
   ${OPENGL_LIBRARIES})
