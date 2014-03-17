#This is an initial CMake configuration file for FluoRender

cmake_minimum_required ( VERSION 2.6 )

project ( FluoRender )

set(ARCHITECTURE 64 CACHE PATH "The architecture to build -- 32 or 64")

set(CMAKE_BUILD_TYPE "Debug")

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

find_package(OpenGL REQUIRED)

#temporary solution for wxWidgets linking "ambiguous" errors
add_definitions(-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_)

#static compile
add_definitions(-DSTATIC_COMPILE)

#output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
   set(CXXFLAGS "-fPIC")

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
      set(CXXFLAGS "-fPIC")
   endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # windows
   if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      add_definitions(-DWIN32)
      add_definitions(-D_WIN32)
      add_definitions(-DSEQAN_C++11_STANDARD=ON)
      set(CFLAGS "")
      set(CXXFLAGS "")
   endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif()


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

subdirs ( fluorender )

