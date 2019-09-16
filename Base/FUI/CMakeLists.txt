cmake_minimum_required(VERSION 3.1.0)

project(FluorenderGUI)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(HEADER_FILES 
    fluoglwidget.hpp
    fui.hpp
    renderview.hpp
    controller.hpp
  )
  
set(SOURCES
    fluoglwidget.cpp
	fui.cpp
	renderview.cpp
	main.cpp
  )
  
set(UI_FILES
    fui.ui
  )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5OpenGL_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
  )

add_executable(FUI
    ${HEADER_FILES}
	${SOURCES}
	${UI_FILES}
  )
  
qt5_use_modules(FUI Core Gui Widgets OpenGL)  

target_link_libraries(FUI
    ${OPENGL_LIBRARIES}
  )