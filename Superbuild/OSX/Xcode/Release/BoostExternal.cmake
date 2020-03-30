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
set( Boost_Bootstrap_Command )

# This is where Boost is hosted, this can be updated to accomodate newer releases of Boost.
# The bootstrap commands are set for each Operating system.
set( Boost_url "https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz")
set( Boost_Hash "SHA256=c66e88d5786f2ca4dbebb14e06b566fb642a1a6947ad8cc9091f9f445134143f")
set( Boost_Bootstrap_Command ./bootstrap.sh )
set( Boost_b2_Command ./b2 )

# I may combine this into one, however I am afraid it may need to be split like this.

# We pass the arguments that it must be built in source, with system, chrono, and filesystem
# The prefix is then set in the Root directory 
ExternalProject_Add(Boost_external_download
  URL ${Boost_url}
  URL_HASH ${Boost_Hash}
  BUILD_IN_SOURCE 1
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ${Boost_Bootstrap_Command}
  BUILD_COMMAND  ${Boost_b2_Command} install
    --with-system
    --with-chrono
    --with-filesystem
    --disable-icu
    --prefix=${CMAKE_BINARY_DIR}/Boost
    --threading=single,multi
    --link=static
    --variant=release
    -j8
  INSTALL_COMMAND ""
  INSTALL_DIR ""
)

# The library directory is cached internally
set(Boost_LIBRARY_DIR ${CMAKE_BINARY_DIR}/Boost/lib CACHE INTERNAL "")

# Windows adds boost-version for some reason, we cache the library and include directories
set(Boost_INCLUDE_DIR ${CMAKE_BINARY_DIR}/Boost/include CACHE INTERNAL "")
set(BOOST_ROOT ${CMAKE_BINARY_DIR}/Boost CACHE INTERNAL "")

ExternalProject_Get_Property(Boost_external_download BINARY_DIR)
SET(Boost_DIR ${BINARY_DIR} CACHE INTERNAL "")

add_library(Boost_external SHARED IMPORTED)

set(Boost_FILESYSTEM_LIBRARY 
  ${Boost_LIBRARY_DIR}/${prefix}boost_filesystem${suffix} 
  CACHE INTERNAL ""
)

set(Boost_SYSTEM_LIBRARY
  ${Boost_LIBRARY_DIR}/${prefix}boost_system${suffix} 
  CACHE INTERNAL ""
)

message(STATUS "Boost_DIR: ${BOOST_ROOT}")
