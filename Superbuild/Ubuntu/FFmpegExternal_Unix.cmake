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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# The github url is set and the latest release tag is set. This project depends
# on x264.
set( FFmpeg_url "https://github.com/FFmpeg/FFmpeg.git")
set( FFmpeg_TAG "n4.2")
set(FFmpeg_depends "x264_external_download")

# we need this so it knows where to find x264's libraries
set(PATH_DEPENDS "${x264_LIBRARY_DIR}/pkgconfig")

# This was difficult to set up, the CONFIGURE_COMMAND is needed in order to tell
# FFmpeg where to find x264's library. Since x264 does not have a Cmake file.
# This file should not be modified unless the developer knows what they are doing.
# The static libraries are built, the linker and the include needs to know where
# x264 is. gpl is enabled and libx264.
ExternalProject_Add(FFmpeg_external_download
  DEPENDS ${FFmpeg_depends}
  GIT_REPOSITORY ${FFmpeg_url}
  GIT_TAG ${FFmpeg_TAG}
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  INSTALL_DIR ""
  CONFIGURE_COMMAND PKG_CONFIG_PATH=${PATH_DEPENDS} <SOURCE_DIR>/configure
    --prefix=<BINARY_DIR>/build
    --enable-static
    --extra-cflags=-I${x264_INCLUDE_DIR}\ --static
    --extra-ldflags=-L${x264_LIBRARY_DIR}
    --enable-gpl
    --enable-libx264
  BUILD_COMMAND make install
    -j8
)

ExternalProject_Get_Property(FFmpeg_external_download BINARY_DIR)

set(FFmpeg_LIBRARY_DIR ${BINARY_DIR}/build/lib CACHE INTERNAL "")
set(FFmpeg_INCLUDE_DIR ${BINARY_DIR}/build/include CACHE INTERNAL "")

# We define the FFmpeg libraries here and cache them internally.
add_library(FFmpeg_external SHARED IMPORTED)
set(FFmpeg_LIBRARIES
  ${FFmpeg_LIBRARY_DIR}/libavutil.a
  ${FFmpeg_LIBRARY_DIR}/libavformat.a
  ${FFmpeg_LIBRARY_DIR}/libavcodec.a
  ${FFmpeg_LIBRARY_DIR}/libavdevice.a
  ${FFmpeg_LIBRARY_DIR}/libavfilter.a
  ${FFmpeg_LIBRARY_DIR}/libpostproc.a
  ${FFmpeg_LIBRARY_DIR}/libswresample.a
  ${FFmpeg_LIBRARY_DIR}/libswscale.a
  CACHE INTERNAL "" 
)

message(STATUS "FFmpeg_DIR: ${FFmpeg_LIBRARY_DIR}")
