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

project(SceneGraph)

# This may not be needed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Includes all the dependency header files
include_directories(${FLObject_INCLUDES}
					${FLIVR_INCLUDE_DIRS}
					${Boost_INCLUDE_DIR}
					${OPENGL_INCLUDE_DIRS}
					${GLEW_INCLUDE_DIR}
					${Teem_INCLUDE_DIR}
					${OpenCL_INCLUDE_DIR}
					${glm_INCLUDE_DIR}
					)

# The headers and source files are manually set rather than Globbed in 
# order to give us better control of what goes in the project.
set(HEADER_FILES Annotations/Annotations.hpp
				Data/MeshData/MeshData.hpp
				Data/VolumeData/VolumeData.hpp
				DrawVolumeVisitor/DrawVolumeVisitor.hpp
				Factories/Annotations/AnnotationFactory.hpp
				Factories/Mesh/MeshFactory.hpp
				Factories/Volume/VolumeFactory.hpp
				Groups/Group.hpp
				Groups/Mesh/MeshGroup.hpp
				Groups/Volume/VolumeGroup.hpp
				Node/Node.hpp
				NodeVisitor/NodeVisitor.hpp
				View/View.hpp
				Root/Root.hpp
				Global/Global.hpp
				Global/Names.hpp
				Processor/ProcessorNode.hpp
				Processor/Processor.hpp
				Processor/ProcessorNodeFactory.hpp
				Processor/ProcessorVisitor.hpp
				Processor/ProcessorFactory.hpp
				Renderer/Renderer2D.hpp
				Renderer/Renderer3D.hpp
				Renderer/ClipPlaneRenderer.hpp
				Renderer/ViewRenderer.hpp
				Renderer/Renderer2DFactory.hpp
				Renderer/Renderer3DFactory.hpp
				Utilities/DecycleVisitor.hpp
				Utilities/InfoVisitor.hpp
				Utilities/Layer.hpp
				Utilities/PopVolumeVisitor.hpp
				Utilities/SearchVisitor.hpp
				Utilities/ValueUpdateVisitor.hpp
				Agent/InterfaceAgent.hpp
	)

set(SOURCES Annotations/Annotations.cpp
			Data/MeshData/MeshData.cpp
			Data/VolumeData/VolumeData.cpp
			DrawVolumeVisitor/DrawVolumeVisitor.cpp
			Factories/Annotations/AnnotationFactory.cpp
			Factories/Mesh/MeshFactory.cpp
			Factories/Volume/VolumeFactory.cpp
			Groups/Group.cpp
			Groups/Mesh/MeshGroup.cpp
			Groups/Volume/VolumeGroup.cpp
			Node/Node.cpp
			NodeVisitor/NodeVisitor.cpp
			View/View.cpp
			Root/Root.cpp
			Global/Global.cpp
			Processor/ProcessorNode.cpp
			Processor/Processor.cpp
			Processor/ProcessorNodeFactory.cpp
			Processor/ProcessorVisitor.cpp
			Processor/ProcessorFactory.cpp
			Renderer/Renderer2D.cpp
			Renderer/Renderer3D.cpp
			Renderer/ClipPlaneRenderer.cpp
			Renderer/ViewRenderer.cpp
			Renderer/Renderer2DFactory.cpp
			Renderer/Renderer3DFactory.cpp
			)


# The headers and library locations are cached interally to be used when
# this library is called.
set(SceneGraph_INCLUDES ${HEADER_FILES} CACHE INTERNAL "")
set(SceneGraph_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")

# This library is built statically
add_library(SceneGraph STATIC ${HEADER_FILES} ${SOURCES}) 

# This library's include directories when this library is used.
target_include_directories(SceneGraph PUBLIC Annotations
	Agent
	Data
	DrawVolumeVisitor
	Factories
	Groups
	Node
	NodeVisitor
	View
	Root
	Utilities
	Global
	Processor
	Renderer
	)

link_directories(SceneGraph ${Teem_DIR}/bin)

add_dependencies(SceneGraph Teem_external_download)

# Links with FlObject, FLIVR, and Boost.
target_link_libraries(SceneGraph FLObject FLIVR ${Boost_LIBRARIES} ${teem_LIBRARIES} ${GLEW_LIBRARIES})

# may not be needed.
install(TARGETS SceneGraph
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
