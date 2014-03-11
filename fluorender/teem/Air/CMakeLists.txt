#This is an initial CMake configuration file for FluoRender
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
add_library(TeemAir 	754.c 
			array.c 	
			dio.c 
			endianAir.c 
			enum.c 
			math.c 
			miscAir.c 	
			mop.c 	
			parseAir.c 
			privateAir.h
			randMT.c
			sane.c
			string.c
			threadAir.c )
