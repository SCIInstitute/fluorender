#This is an initial CMake configuration file for FluoRender
if(${ARCHITECTURE} MATCHES 64)
   add_definitions(-DTEEM_32BIT=0)
else()
   add_definitions(-DTEEM_32BIT=1)
endif()

add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)

file(GLOB airsrc Air/*.c)
file(GLOB airhdr Air/*.h)
file(GLOB hestsrc Hest/*.c)
file(GLOB hesthdr Hest/*.h)
file(GLOB biffsrc Biff/*.c)
file(GLOB biffhdr Biff/*.h)
file(GLOB nrrdsrc Nrrd/*.c)
file(GLOB nrrdhdr Nrrd/*.h)

add_library(Teem 
   ${airsrc} ${hestsrc} ${nrrdsrc} ${biffsrc}
   ${airhdr} ${hesthdr} ${nrrdhdr} ${biffhdr}) 
