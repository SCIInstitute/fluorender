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

# The latest release is downloaded from FFmpeg's website directly. 
set( FFmpeg_url "https://github.com/vot/ffbinaries-prebuilt/releases/download/v4.2/ffmpeg-4.2-osx-64.zip")

# This is a little annoying, Ninja needs to know exactly where the libraries will be placed 
# or it will not build. Oddly, this is the only file that needs this. 
ExternalProject_Add(FFmpeg_external_download
  URL ${FFmpeg_url}
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  INSTALL_DIR ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)


ExternalProject_Get_Property(FFmpeg_external_download SOURCE_DIR)

set(FFmpeg_LIBRARY_DIR ${SOURCE_DIR}/lib CACHE INTERNAL "")
set(FFmpeg_INCLUDE_DIR ${SOURCE_DIR}/include CACHE INTERNAL "")

add_library(FFmpeg_external STATIC IMPORTED)

# The libraries are set manually and cached internally 
set(FFmpeg_LIBRARIES
  ${FFmpeg_LIBRARY_DIR}/${prefix}avutil${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avformat${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avcodec${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avdevice${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avfilter${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}postproc${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}swresample${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}swscale${suffix}
  CACHE INTERNAL "" 
)
message(STATUS "FFmpeg_DIR: ${FFmpeg_LIBRARY_DIR}")
