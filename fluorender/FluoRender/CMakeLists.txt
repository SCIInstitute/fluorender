#This is an initial CMake configuration file for FluoRender

include_directories(${FluoRender_SOURCE_DIR}/fluorender)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew/GL)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/LibTiff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Formats)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Animators)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/NV)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/FLIVR)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/Converters)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/img)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   # Windows
   include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender/WacUtils)
   include_directories(${FluoRender_SOURCE_DIR}/fluorender/Wacom/Include)

   file(GLOB wac_src WacUtils/*.cpp)
   file(GLOB wac_hdr WacUtils/*.h)
endif()

file(GLOB src *.cpp)
file(GLOB hdr *.h)
file(GLOB rsc img/*.xpm)

file(GLOB pngs img/*.h)

file(GLOB ani_src Animator/*.cpp)
file(GLOB ani_hdr Animator/*.h)

file(GLOB cvt_src Converters/*.cpp)
file(GLOB cvt_hdr Converters/*.h)

file(GLOB fli_src FLIVR/*.cpp)
file(GLOB fli_hdr FLIVR/*.h)

file(GLOB fmt_src Formats/*.cpp)
file(GLOB fmt_hdr Formats/*.h)

file(GLOB nv_src NV/*.cpp)
file(GLOB nv_hdr NV/*.h)

link_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
link_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X
   include(BundleUtilities)
   add_executable(FluoRender MACOSX_BUNDLE 
      ${src} ${hdr} ${rsc} ${pngs} 
      ${ani_src} ${ani_hdr} 
      ${cvt_src} ${cvt_hdr} 
      ${fli_src} ${fli_hdr} 
      ${fmt_src} ${fmt_hdr} 
      ${nv_src} ${nv_hdr} 
      ${wac_src} ${wac_hdr}) 


elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # Linux 
   add_executable(FluoRender 
      ${src} ${hdr} ${rsc} ${pngs}
      ${ani_src} ${ani_hdr} 
      ${cvt_src} ${cvt_hdr} 
      ${fli_src} ${fli_hdr} 
      ${fmt_src} ${fmt_hdr} 
      ${nv_src} ${nv_hdr} 
      ${wac_src} ${wac_hdr}) 
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   # Windows
   add_executable(FluoRender 
      ${src} ${hdr} ${rsc} ${pngs}
      ${ani_src} ${ani_hdr} 
      ${cvt_src} ${cvt_hdr} 
      ${fli_src} ${fli_hdr} 
      ${fmt_src} ${fmt_hdr} 
      ${nv_src} ${nv_hdr} 
      ${wac_src} ${wac_hdr}) 
endif()

target_link_libraries(FluoRender 
   Teem Glew
   ${wxWidgets_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${GLUT_LIBRARY})

