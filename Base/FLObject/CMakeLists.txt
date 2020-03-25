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


project(FLObject)

# this might not be needed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# This probably can be combined. SceneGraph_INCLUDES is defined in SceneGraph since 
# FLObject and SceneGraph depend on each other. Hopefully this will be fixed in the 
# future
include_directories(${SceneGraph_INCLUDES})
include_directories(${Boost_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} ${Teem_INCLUDE_DIR} ${OpenCL_INCLUDE_DIR})

# We manually set the Headers and Source files instead of Globbing them. This gives us
# better control of what goes in the project and what does not. 
set(HEADER_FILES Event/Event.hpp
                 EventHandler/EventHandler.hpp
                 Object/CopyOp.hpp
                 Object/Object.hpp
                 Object/ObjectFactory.hpp
                 Observer/Observer.hpp
                 Referenced/Referenced.hpp
                 ReferencePtr/ref_ptr.hpp
                 Value/Value.hpp
                 Types/BBox.hpp
                 Types/Color.hpp
                 Types/GLfloat4.hpp
                 Types/GLint4.hpp
                 Types/Plane.hpp
                 Types/Point.hpp
                 Types/Quaternion.hpp
                 Types/Ray.hpp
                 Types/Transform.hpp
                 Types/Utils.hpp
                 Types/Vector.hpp)

set(SOURCES Event/Event.cpp
            Object/CopyOp.cpp
            Object/Object.cpp
            Object/ObjectFactory.cpp
            Observer/Observer.cpp
            Referenced/Referenced.cpp
            Value/Value.cpp
            Types/BBox.cpp
            Types/Color.cpp
            Types/Plane.cpp
            Types/Point.cpp
            Types/Ray.cpp
            Types/Transform.cpp
            Types/Vector.cpp)

# FLObject includes and the library are cached internally since SceneGraph and 
# others depend on this library.
set(FLObject_INCLUDES ${HEADER_FILES} CACHE INTERNAL "")
set(FLObject_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")

# We create FLObject as a static library
add_library(FLObject STATIC ${HEADER_FILES} ${SOURCES}) 

# We define what directories are to be included when you import this library.
target_include_directories(FLObject PUBLIC Event
                                           EventHandler
                                           Object
                                           Observer
                                           Referenced
                                           ReferencePtr
                                           Value
                                           Types)

# This library depends on Booost and Glew.
add_dependencies(FLObject Boost_external_download glew_external_download Teem_external_download)

# We link with SceneGraph and Boost
target_link_libraries(FLObject SceneGraph ${teem_LIBRARIES} ${Boost_LIBRARIES} ${GLEW_LIBRARIES})

# I do not think this is needed.
install(TARGETS FLObject
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
