#This is an initial CMake configuration file for FluoRender

include_directories(${FluoRender_SOURCE_DIR}/fluorender/LibTiff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender)

add_library(LibFormats 	base_reader.cpp
			lbl_reader.cpp
			lsm_reader.cpp
			msk_reader.cpp
			msk_writer.cpp
			nrrd_reader.cpp
			nrrd_writer.cpp
			oib_reader.cpp
			oif_reader.cpp
			png_resource.cpp
			pvxml_reader.cpp
			tif_reader.cpp
			tif_writer.cpp )
