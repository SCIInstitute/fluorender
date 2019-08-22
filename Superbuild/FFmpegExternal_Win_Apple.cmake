
SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

if( WIN32 )
  set( FFmpeg_url "http://ffmpeg.zeranoe.com/builds/win64/dev/ffmpeg-4.2-win64-dev.zip")
elseif( APPLE )
  set( FFmpeg_url "https://ffmpeg.zeranoe.com/builds/macos64/dev/ffmpeg-4.2-macos64-dev.zip")
endif()

ExternalProject_Add(FFmpeg_external_download
  URL ${FFmpeg_url}
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  INSTALL_DIR ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)

#CACHE PATH "" seems to write the path to a file that I can set 
#library paths to. 

ExternalProject_Get_Property(FFmpeg_external_download SOURCE_DIR)

set(FFmpeg_LIBRARY_DIR ${SOURCE_DIR}/lib CACHE INTERNAL "")
set(FFmpeg_INCLUDE_DIR ${SOURCE_DIR}/include CACHE INTERNAL "")

if(WIN32)
  if(MSVC)
    set(prefix "")
    set(suffix ".lib")
  endif()
  if(MINGW)
    set(prefix "lib")
    set(suffix ".dll.a")
  endif()
elseif(APPLE)
  set(prefix "lib")
  set(suffix ".a")
endif()

add_library(FFmpeg_external STATIC IMPORTED)

set(FFmpeg_LIBRARIES
  ${FFmpeg_LIBRARY_DIR}/${prefix}avutil${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avformat${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avcodec${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avdevice${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}avfilter${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}postproc${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}swresample${suffix}
  ${FFmpeg_LIBRARY_DIR}/${prefix}swscale${suffix}
  CACHE INTERNAL "" 
)
message(STATUS "FFmpeg_DIR: ${FFmpeg_LIBRARY_DIR}")
