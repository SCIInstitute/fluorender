#This is an initial CMake configuration file for FluoRender

cmake_minimum_required ( VERSION 2.6 )

set(ARCHITECTURE 64 CACHE PATH "The architecture to build -- 32 or 64")

project ( FluoRender )

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base aui gl media html xrc xml net adv REQUIRED)
include(${wxWidgets_USE_FILE})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

#temporary solution for wxWidgets linking "ambiguous" errors
add_definitions(-D_WCHAR_H_CPLUSPLUS_98_CONFORMANCE_)

#static compile
add_definitions(-DSTATIC_COMPILE)

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
   endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # windows
   if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
      add_definitions(-DWIN32)
      add_definitions(-D_WIN32)
      set (CXX_11_FLAG "-std=c++11")
   endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif()

set(CFLAGS "-fPIC")
set(CXXFLAGS "-fPIC -I/usr/local/lib/wx/include/osx_cocoa-unicode-3.0 -I/usr/local/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXMAC__ -D__WXOSX__ -D__WXOSX_COCOA__")

if(${ARCHITECTURE} MATCHES 64)
   set(ARCH_FLAGS "-m64 -arch x86_64")
   add_definitions(-DTEEM_32BIT=0)
   set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}")
   set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
   set(LDFLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
else()
   set(ARCH_FLAGS "-m32 -arch i386")
   add_definitions(-DTEEM_32BIT=1)
   set(CMAKE_C_FLAGS "${ARCH_FLAGS} ${CFLAGS}" )
   set(CMAKE_CXX_FLAGS "${ARCH_FLAGS} ${CXXFLAGS} ${CXX_11_FLAG}")
   set(LDFLAGS "${ARCH_FLAGS} ${CXX_11_FLAG}")
endif()

add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

subdirs ( fluorender )

