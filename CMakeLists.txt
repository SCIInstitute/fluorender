cmake_minimum_required(VERSION 3.0)
project(Fluorender)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

if(UNIX)
  find_program(NASM_EXE NAMES nasm)
  
  if(NOT NASM_EXE)
    MESSAGE(FATAL_ERROR "
      nasm was not found on your system and is required for FFmpeg, please install with: 
      
      - sudo apt update
      - sudo apt install nasm 
    
      then run Cmake again. 
    ")
  endif()
endif()

if(WIN32)
  include(getCompiler.cmake)
endif()

include(setPreSuffix.cmake)

# Required packages
find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Java COMPONENTS Development REQUIRED)
find_package(JNI REQUIRED)

if(${OpenCL_FOUND})
  message(STATUS "Found OpenCL")
endif()

#headers
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5OpenGL_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${JNI_INCLUDE_DIRS})
include_directories(Base)
include_directories(InternalDepends)
include_directories(Processor)
include_directories(Renderer)

#defines
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5OpenGL_DEFINITIONS})
add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Gui_DEFINITIONS})

Include(${CMAKE_SOURCE_DIR}/Superbuild/Superbuild.cmake)

set(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIR})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Teem_INCLUDE_DIR})
include_directories(${FFmpeg_INCLUDE_DIR})
include_directories(${glm_INCLUDE_DIR})
include_directories(${pole_INCLUDE_DIR})
include_directories(${libtiff_INCLUDE_DIR})

add_subdirectory(InternalDepends)
add_subdirectory(Base)

#add_subdirectory(Processor)
#add_subdirectory(Renderer)
add_subdirectory(Tester)
