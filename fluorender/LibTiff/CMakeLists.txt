#This is an initial CMake configuration file for FluoRender

cmake_minimum_required (VERSION 2.6)

project (FluoRender)

add_executable (FluoRender fluorender/glew/*.c 
                           fluorender/teem/*/*.c 
                           fluorender/FluoRender/*.cpp 
			   fluorender/FluoRender/*/*.cpp)

include_directories ( fluorender/glew 
		      fluorender/LibTiff 
		      fluorender/wxWidgets/include 
		      fluorender/wxWidgets/include/msvc 
		      fluorender/teem 
	 	      fluorender/teem/air 
	 	      fluorender/teem/hest 
	 	      fluorender/teem/biff 
	 	      fluorender/teem/nrrd 
		      fluorender/Wacom/include )

target_link_libraries ( FluoRender wx_osx_cocoau_xrc-3.0 
				   wx_osx_cocoau_webview-3.0 
				   wx_osx_cocoau_html-3.0 
				   wx_osx_cocoau_qa-3.0 
				   wx_osx_cocoau_adv-3.0 
				   wx_osx_cocoau_core-3.0 
				   wx_baseu_xml-3.0 
				   wx_baseu_net-3.0 wx_baseu-3.0 )

install (TARGETS FluoRender DESTINATION bin)
