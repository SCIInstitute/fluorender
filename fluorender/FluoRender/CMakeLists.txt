#This is an initial CMake configuration file for FluoRender

include_directories(${FluoRender_SOURCE_DIR}/fluorender)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/LibTiff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Animator)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Converters)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/FLIVR)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Formats)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/NV)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X
   subdirs(Animator Converters FLIVR Formats NV )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # Linux 
   subdirs(Animator Converters FLIVR Formats NV )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   # Windows
   include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/WacUtils)
   include_directories(${FluoRender_SOURCE_DIR}/fluorender/Wacom/Include)
   subdirs(Animator Converters FLIVR Formats NV WacUtils)
endif()



link_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)

add_library(Main   
   AdjustView.cpp
   APropView.cpp
   bitmap_fonts.cpp
   BrushToolDlg.cpp
   ClippingView.cpp
   ColocalizationDlg.cpp
   ConvertDlg.cpp
   CountingDlg.cpp
   DataManager.cpp
   DragDrop.cpp
   HelpDlg.cpp
   ListPanel.cpp
   MeasureDlg.cpp
   MManipulator.cpp
   MPropView.cpp
   NoiseCancellingDlg.cpp
   RecorderDlg.cpp
   SettingDlg.cpp
   Tester.cpp
   TraceDlg.cpp
   TreePanel.cpp
   VMovieView.cpp
   VolumeCalculator.cpp
   VolumeSelector.cpp
   VPropView.cpp
   VRenderFrame.cpp
   VRenderView.cpp )

add_executable(FluoRender Main.cpp)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X
target_link_libraries(FluoRender  Main
   TeemAir
   TeemHest
   TeemBiff
   TeemNrrd
   Glew
   Animator
   Converters
   FLIVR
   Formats
   NV
   ${wxWidgets_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${GLUT_LIBRARY})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # Linux 
target_link_libraries(FluoRender  Main
   TeemAir
   TeemHest
   TeemBiff
   TeemNrrd
   Glew
   Animator
   Converters
   FLIVR
   Formats
   NV
   ${wxWidgets_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${GLUT_LIBRARY})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   # Windows
target_link_libraries(FluoRender  Main
   TeemAir
   TeemHest
   TeemBiff
   TeemNrrd
   Glew
   Animator
   Converters
   FLIVR
   Formats
   NV
   WacUtils
   ${wxWidgets_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${GLUT_LIBRARY})
endif()

