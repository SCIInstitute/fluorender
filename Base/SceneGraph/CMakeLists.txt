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
set(HEADER_FILES
				Agent/InterfaceAgent.hpp
				Data/Annotations.hpp
				Data/MeshData.hpp
				Data/VolumeData.hpp
				Data/AnnotationFactory.hpp
				Data/MeshFactory.hpp
				Data/VolumeFactory.hpp
				Data/MeshGroup.hpp
				Data/VolumeGroup.hpp
				Data/View.hpp
				Data/Root.hpp
				Data/Layer.hpp
				Global/Global.hpp
				Global/Names.hpp
				Node/Node.hpp
				Node/Group.hpp
				Node/NodeVisitor.hpp
				Node/DecycleVisitor.hpp
				Node/InfoVisitor.hpp
				Node/PopVolumeVisitor.hpp
				Node/SearchVisitor.hpp
				Node/ValueUpdateVisitor.hpp
				Processor/ProcessorNode.hpp
				Processor/Processor.hpp
				Processor/ProcessorNodeFactory.hpp
				Processor/ProcessorVisitor.hpp
				Processor/ProcessorFactory.hpp
				Renderer/render_defs.hpp
				Renderer/Renderer2D.hpp
				Renderer/Renderer3D.hpp
				Renderer/ClipPlaneRenderer.hpp
				Renderer/Renderer2DFactory.hpp
				Renderer/Renderer3DFactory.hpp
				Renderer/ViewRenderer.hpp
				Renderer/DrawSimple.hpp
				Renderer/DrawPeel.hpp
				Renderer/DrawBackground.hpp
				Renderer/DrawGrid.hpp
				Renderer/DrawVolume.hpp
				Renderer/DrawBound.hpp
				Renderer/DrawAnnotation.hpp
				Renderer/DrawRuler.hpp
				Renderer/DrawCell.hpp
				Renderer/DrawTrack.hpp
				Renderer/DrawVolumeVisitor.hpp
				Renderer/DrawMesh.hpp
	)

set(SOURCES Data/Annotations.cpp
			Data/MeshData.cpp
			Data/VolumeData.cpp
			Data/AnnotationFactory.cpp
			Data/MeshFactory.cpp
			Data/VolumeFactory.cpp
			Data/MeshGroup.cpp
			Data/VolumeGroup.cpp
			Data/View.cpp
			Data/Root.cpp
			Node/Node.cpp
			Node/Group.cpp
			Node/NodeVisitor.cpp
			Global/Global.cpp
			Processor/ProcessorNode.cpp
			Processor/Processor.cpp
			Processor/ProcessorNodeFactory.cpp
			Processor/ProcessorVisitor.cpp
			Processor/ProcessorFactory.cpp
			Renderer/Renderer2D.cpp
			Renderer/Renderer3D.cpp
			Renderer/ClipPlaneRenderer.cpp
			Renderer/Renderer2DFactory.cpp
			Renderer/Renderer3DFactory.cpp
			Renderer/ViewRenderer.cpp
			Renderer/DrawSimple.cpp
			Renderer/DrawPeel.cpp
			Renderer/DrawBackground.cpp
			Renderer/DrawGrid.cpp
			Renderer/DrawVolume.cpp
			Renderer/DrawBound.cpp
			Renderer/DrawAnnotation.cpp
			Renderer/DrawRuler.cpp
			Renderer/DrawCell.cpp
			Renderer/DrawTrack.cpp
			Renderer/DrawVolumeVisitor.cpp
			Renderer/DrawMesh.cpp
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
	Global
	Node
	Processor
	Renderer
	Utilities
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
