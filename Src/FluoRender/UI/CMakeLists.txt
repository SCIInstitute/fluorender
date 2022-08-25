# Note the qt_add_* functions come from Qt6 Core component.

file(GLOB ui_src
	  Interface/*.cpp
	  MainWindow/*.cpp
	  Preferences/*.cpp
	  Properties/*.cpp
	  Tools/*.cpp
	  ViewWindow/*.cpp
	  Widgets/*.cpp
	  )

file(GLOB ui_hdr
	  Interface/*.h
	  MainWindow/*.h
	  Preferences/*.h
	  Properties/*.h
	  Tools/*.h
	  ViewWindow/*.h
	  Widgets/*.h
	  )

file(GLOB ui_ui
	  Interface/*.ui
	  MainWindow/*.ui
	  Preferences/*.ui
	  Properties/*.ui
	  Tools/*.ui
	  ViewWindow/*.ui
	  Widgets/*.ui
	  )

qt_add_library(FluoRenderUI SHARED ${ui_src} ${ui_hdr} ${ui_ui})

target_include_directories(FluoRenderUI PUBLIC # Should it be private??
    ${FLUORENDER_COMMON_INCLUDES}
    ${qtadvanceddocking_INCLUDE_DIR}
    Interface
    MainWindow
    Preferences
    Properties
    Tools
    ViewWindow
    Widgets
)

target_link_directories(FluoRenderUI PUBLIC # Should it be private??
    ${qtadvanceddocking_LIBRARY_DIR}
)
    
target_link_libraries(FluoRenderUI PUBLIC # Should it be private??
    ${qtadvanceddocking_LIBRARIES}
    ${QT_LIBRARIES}
)

# Resources:
set(FluoRender_resource_files
    "icons/FluoRenderIcon.gif"
    "icons/FluoRenderIcon.icns"
    "icons/MainWindow/OpenMesh.png"
    "icons/MainWindow/OpenProject.png"
    "icons/MainWindow/OpenVolume.png"
    "icons/MainWindow/Preferences.png"
    "icons/MainWindow/SaveProject.png"
    "icons/MainWindow/Visibility.png"
    "icons/Tools/Calculation.png"
    "icons/Tools/Colocalization.png"
    "icons/Tools/ComponentAnalyzer.png"
    "icons/Tools/Convert.png"
    "icons/Tools/Measurement.png"
    "icons/Tools/NoiseReduction.png"
    "icons/Tools/OpenCLKernelEditor.png"
    "icons/Tools/PaintBrush.png"
    "icons/Tools/Tracking.png"
    "icons/Tools/VolumeSize.png"
    "icons/animate/animate.xpm"
    "icons/animate/animationforwardstep.xpm"
    "icons/animate/animationplayoff.xpm"
    "icons/animate/animationplayon.xpm"
    "icons/animate/animationreverseplayoff.xpm"
    "icons/animate/animationreverseplayon.xpm"
    "icons/animate/animationreversestep.xpm"
    "icons/animate/animationstopoff.xpm"
    "icons/animate/animationstopon.xpm"
    "icons/misc/CircleBackground.png"
    "icons/misc/CircleBase.png"
    "icons/misc/LightBackground.png"
    "icons/misc/LightBase.png"
    "icons/misc/LightOff.png"
    "icons/misc/LightOn.png"
    "icons/misc/Mask.png"
    "icons/misc/TrashcanOutline.png"
    "icons/misc/TrashcanSolid.svg"
    "images/copy.png"
    "images/cut.png"
    "images/new.png"
    "images/open.png"
    "images/paste.png"
    "images/save.png"
)

qt6_add_resources(FluoRenderUI "FluoRender"
    PREFIX
        "/"
    FILES
        ${FluoRender_resource_files}
)

