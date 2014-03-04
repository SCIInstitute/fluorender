#This is an initial CMake configuration file for FluoRender

include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/Wacom/Include)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/LibTiff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Animator)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Converters)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/FLIVR)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Formats)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/NV)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/WacUtils)

subdirs(Animator Converters FLIVR Formats NV WacUtils)


link_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)

add_library(LibMain 	AdjustView.cpp
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
			utility.cpp
			VMovieView.cpp
			VolumeCalculator.cpp
			VolumeSelector.cpp
			VPropView.cpp
			VRenderFrame.cpp
			VRenderView.cpp )

add_executable(FluoRender Main.cpp)

target_link_libraries(FluoRender	LibMain 
					LibTeemAir
					LibTeemHest
					LibTeemBiff
					LibTeemNrrd 
					LibGlew 
					LibAnimator 
					LibConverters 
					LibFLIVR 
					LibFormats
					LibNV
					LibWacUtils 
					${wxWidgets_LIBRARIES})
