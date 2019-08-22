#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software. 
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
SET(teem_GIT_TAG "origin/master")
SET(teem_DEPENDENCIES "Zlib_external_download;LibPNG_external_download")

set(zlibincludedir "${Zlib_LIBRARY_DIR};${Zlib_INCLUDE_DIR}")
set(libpnginclude "${LibPNG_LIBRARY_DIR};${LibPNG_INCLUDE_DIR}")
set(Master_Depends ${Zlib_LIBRARY_DIR} ${LibPNG_LIBRARY_DIR})

string(REPLACE ";" "|" Master_Root "${Master_Depends}")


# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Teem_external_download
  DEPENDS ${teem_DEPENDENCIES}
  GIT_REPOSITORY "https://github.com/Sailanarmo/teem.git"
  GIT_TAG ${teem_GIT_TAG}
  PATCH_COMMAND ""
  INSTALL_DIR ""
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  LIST_SEPARATOR |
  CMAKE_ARGS ${Teem_external_download_CMAKE_ARGS} 
    -DCMAKE_PREFIX_PATH=${Master_Root}
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    -DZlib_DIR:PATH=${Zlib_DIR}
    -DLibPNG_DIR:PATH=${LibPNG_DIR}
    -DTeem_USE_NRRD_INTERNALS:BOOL=ON
    -DZLIB_INCLUDE_DIR:PATH=${Zlibincludes}
    -DPNG_INCLUDE_DIR:PATH=${libpnginclude}
	-DPNG_PNG_INCLUDE_DIR:PATH=${libpnginclude}
)

ExternalProject_Get_Property(Teem_external_download BINARY_DIR)

if(MSVC)
  SET(Teem_DIR "${BINARY_DIR};${BINARY_DIR}/Debug;${BINARY_DIR}/Release" CACHE INTERNAL "")
else()
  SET(Teem_DIR ${BINARY_DIR} CACHE INTERNAL "")
endif()

set(Teem_INCLUDE_DIR "${BINARY_DIR}/include" CACHE INTERNAL "")

add_library(Teem_external STATIC IMPORTED)

MESSAGE(STATUS "Teem_DIR: ${Teem_DIR}")
