#This is an initial CMake configuration file for FluoRender

cmake_minimum_required ( VERSION 2.6 )

set(ARCHITECTURE 64 CACHE PATH "The architecture to build -- 32 or 64")

project ( FluoRender )

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

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
endif()

if(${ARCHITECTURE} MATCHES 64)
	add_definitions(-DTEEM_32BIT=0)
else()
	add_definitions(-DTEEM_32BIT=1)
endif()

add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

add_definitions(-DOPEN_GLU_HEADER=<GL/glu.h>)
add_definitions(-DOPEN_GLU_HEADER=<GL/glu.h>)

subdirs ( fluorender )

