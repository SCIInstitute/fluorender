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

# I believe this sets where the superbuild folder will be build and set
# does not seem to work sometimes

#TODO: Edit the glew file to download from another repo like Teem. It seems that 
#      sometimes glew will cause an error where it cannot link with glew.lib because
#      it cannot be built. Commenting out two lines seems to do the trick.
#      See this link: https://github.com/nigels-com/glew/issues/99
#      Comment out lines 98 and 103.

SET( Source_DEPENDENCIES )
SET(ep_base "${CMAKE_BINARY_DIR}/Superbuild" CACHE INTERNAL "")

# This is a custom defined Macro, to be honest, I do not really know 
# what it does other than make the files down below work.
MACRO(ADD_EXTERNAL cmake_file external)
  INCLUDE( ${cmake_file} )
  LIST(APPEND Source_DEPENDENCIES ${external})
ENDMACRO()

# this is needed for ExternalProject
include(ExternalProject)

# This will need to be tested, however I believe the already built binaries for
# Windows and OSX have x264 built into them. However there are not pre-built 
# binaries for Linux so it must be manually built.
if( APPLE )
  ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/FFmpegExternal_Win_Apple.cmake FFmpeg_external)
elseif( UNIX )
  ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/x264_external.cmake x264_external)
  ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/FFmpegExternal_Unix.cmake FFmpeg_external)
else()
  ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/FFmpegExternal_Win_Apple.cmake FFmpeg_external)
endif()

# This adds all of external projects to be added to the project
#
# TODO: Check to see if Boost is already installed or not. No need to include
#       Boost as an external project if the user already has a version of 
#       boost. However, this may break some dependencies down the line and 
#       some of the projects that depends on Boost_external_download will
#       need to be modified.
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/ZlibExternal.cmake Zlib_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/LibPNGExternal.cmake LibPNG_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/libtiffExternal.cmake LibPNG_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/TeemExternal.cmake Teem_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/freetypeExternal.cmake freetype_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/glmExternal.cmake glm_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/BoostExternal.cmake Boost_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/poleExternal.cmake pole_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/GlewExternal.cmake glew_external)
ADD_EXTERNAL(${CMAKE_CURRENT_LIST_DIR}/pugiXMLExternal.cmake pugi_external)
