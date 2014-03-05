#This is an initial CMake configuration file for FluoRender

cmake_minimum_required ( VERSION 2.6 )

set(ARCHITECTURE 64 CACHE PATH "The architecture to build -- 32 or 64")

project ( FluoRender )

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

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
elseif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Linux or Windows
	if(NOT OPEN_GL_HEADER_LOC)
		add_definitions(-DOPEN_GL_HEADER_LOC=<GL/gl.h>)
	endif()
	if(NOT OPEN_GLU_HEADER_LOC)
		add_definitions(-DOPEN_GLU_HEADER_LOC=<GL/glu.h>)
	endif()
        # windows
        if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
           add_definitions(-DWIN32)
        endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif()

if(${ARCHITECTURE} MATCHES 64)
	add_definitions(-DTEEM_32BIT=0)
else()
	add_definitions(-DTEEM_32BIT=1)
        set(CMAKE_C_FLAGS "-m32")
        set(CMAKE_CXX_FLAGS "-m32")
        set(LDFLAGS "-m32")
endif()

add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

subdirs ( fluorender )

