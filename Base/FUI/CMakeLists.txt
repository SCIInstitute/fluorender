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

# This will be removed in the future since we have already defined this
cmake_minimum_required(VERSION 3.1.0)

project(FluorenderGUI)


# This finds the .ui and .rc files defined for QT. It allows us to use the
# GUI tools that are provided in QT and automatically updates them
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# These are manually defined rather than Globbed to give us better control
# of what goes in the project.
set(HEADER_FILES 
    fluoglwidget.hpp
    fui.hpp
    renderview.hpp
    controller.hpp
    FluoToolbar/genToolbarObjects.hpp
    FluoToolbar/renderviewBottomTB.hpp
    FluoToolbar/renderviewTopTB.hpp
    FluoToolbar/renderviewLeftTB.hpp
    FluoToolbar/renderviewRightTB.hpp
    renderCanvas.hpp
    testTriangle.hpp
    CustomWidgets/fluoSlider.hpp
    CustomWidgets/fluoSliderStyle.hpp
    CustomWidgets/fluoSpinbox.hpp
    CustomWidgets/fluoSpinboxDouble.hpp
    CustomWidgets/fluoToolButton.hpp
    CustomWidgets/fluoLine.hpp
    CustomWidgets/fluoColoredLine.hpp
    Panels/Properties/propertiesPanel.hpp
    Panels/Properties/volumePropertiesOptions.hpp
    Panels/Properties/meshPropertiesOptions.hpp
    Panels/Properties/meshPropertiesMaterials.hpp
    Panels/Properties/volumePropertiesMisc.hpp
		Panels/Properties/Agent/volumePropAgent.hpp
    Panels/Properties/Messenger/propertyMessenger.hpp
    Panels/Output_Adjustments/outputAdjust.hpp
    Panels/Output_Adjustments/outputLayout.hpp
    Panels/Clipping_Planes/clippingPlane.hpp
    Panels/Clipping_Planes/clippingLayout.hpp
    Panels/Datasets/datasetsPanel.hpp
    Panels/Datasets/datasetsToolbarLayout.hpp
    Panels/Datasets/datasetsTreeLayout.hpp
    Panels/Workspace/workspaceTreeLayout.hpp
    Panels/Workspace/workspaceToolLayout.hpp
    Panels/Workspace/workspacePanel.hpp
    Panels/Clipping_Planes/clippingPlane.hpp
    Panels/Clipping_Planes/clippingLayout.hpp
    Panels/Record_Export/recordExport.hpp
		Panels/Record_Export/recordTabLayout.hpp
		Panels/Record_Export/recordPlayLayout.hpp
    Panels/Record_Export/Pages/basicPage.hpp
    Panels/Record_Export/Pages/advancedPage.hpp
    Panels/Record_Export/Pages/autoKeyPage.hpp
    Panels/Record_Export/Pages/croppingPage.hpp
    Panels/Record_Export/Pages/4DScriptPage.hpp
		Panels/Base_Agent/AgentFactory.hpp
		Panels/Base_Agent/InterfaceAgent.hpp
  )
  
set(SOURCES
    fluoglwidget.cpp
	  fui.cpp
	  renderview.cpp
	  main.cpp
    FluoToolbar/genToolbarObjects.cpp
    FluoToolbar/renderviewBottomTB.cpp
    FluoToolbar/renderviewTopTB.cpp
    FluoToolbar/renderviewLeftTB.cpp
    FluoToolbar/renderviewRightTB.cpp
    renderCanvas.cpp
    testTriangle.cpp
    Panels/Properties/propertiesPanel.cpp
    Panels/Properties/volumePropertiesOptions.cpp
    Panels/Properties/meshPropertiesOptions.cpp
    Panels/Properties/meshPropertiesMaterials.cpp
    Panels/Properties/volumePropertiesMisc.cpp
		Panels/Properties/Agent/volumePropAgent.cpp
    Panels/Properties/Messenger/propertyMessenger.cpp
    Panels/Output_Adjustments/outputLayout.cpp
    Panels/Workspace/workspaceTreeLayout.cpp
    Panels/Workspace/workspaceToolLayout.cpp
    Panels/Datasets/datasetsToolbarLayout.cpp
    Panels/Datasets/datasetsTreeLayout.cpp
    Panels/Record_Export/recordExport.cpp
		Panels/Record_Export/recordTabLayout.cpp
		Panels/Record_Export/recordPlayLayout.cpp
    Panels/Record_Export/Pages/basicPage.cpp
    Panels/Record_Export/Pages/advancedPage.cpp
    Panels/Record_Export/Pages/autoKeyPage.cpp
    Panels/Record_Export/Pages/croppingPage.cpp
    Panels/Record_Export/Pages/4DScriptPage.cpp
		Panels/Base_Agent/AgentFactory.cpp
  )
  
set(UI_FILES
    FUI.ui
  )

set(RESOURCE_FILE
    resources.qrc
)

# For everything QT Related 
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
	  ${Qt5Widgets_INCLUDE_DIRS}
	  ${Qt5OpenGL_INCLUDE_DIRS}
	  ${Qt5Gui_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Panels
		${FLObject_INCLUDES}
    ${SceneGraph_INCLUDES}
    ${Boost_INCLUDE_DIR}
  )

add_executable(FUI
    ${HEADER_FILES}
	  ${SOURCES}
	  ${UI_FILES}
    ${RESOURCE_FILE}
  )
 
# This works like target_link_libraries, however this is how CMake can take
# care of this automatically.
qt5_use_modules(FUI Core Gui Widgets OpenGL)

add_dependencies(FUI FLObject SceneGraph Boost_external_download)

# links with OpenGL
target_link_libraries(FUI
    FLObject
    SceneGraph
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
  )
