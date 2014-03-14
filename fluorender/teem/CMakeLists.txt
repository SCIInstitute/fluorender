#This is an initial CMake configuration file for FluoRender
if(${ARCHITECTURE} MATCHES 64)
   add_definitions(-DTEEM_32BIT=0)
else()
   add_definitions(-DTEEM_32BIT=1)
endif()

add_definitions(-DTEEM_DIO=0)
add_definitions(-DTEEM_ENDIAN=1234)
add_definitions(-DTEEM_QNANHIBIT=1)

subdirs(Air Biff Hest Nrrd)
