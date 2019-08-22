SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

set( FFmpeg_url "https://github.com/FFmpeg/FFmpeg.git")
set( FFmpeg_TAG "n4.2")
set(FFmpeg_depends "x264_external_download")

# we need this so it knows where to find x264's libraries
set(PATH_DEPENDS "${x264_LIBRARY_DIR}/pkgconfig")

ExternalProject_Add(FFmpeg_external_download
  DEPENDS ${FFmpeg_depends}
  GIT_REPOSITORY ${FFmpeg_url}
  GIT_TAG ${FFmpeg_TAG}
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  INSTALL_DIR ""
  CONFIGURE_COMMAND PKG_CONFIG_PATH=${PATH_DEPENDS} <SOURCE_DIR>/configure
    --prefix=<BINARY_DIR>/build
    --enable-static
    --extra-cflags=-I${x264_INCLUDE_DIR}\ --static
    --extra-ldflags=-L${x264_LIBRARY_DIR}
    --enable-gpl
    --enable-libx264
  BUILD_COMMAND make install
    -j8
)

ExternalProject_Get_Property(FFmpeg_external_download BINARY_DIR)

set(FFmpeg_LIBRARY_DIR ${BINARY_DIR}/build/lib CACHE INTERNAL "")
set(FFmpeg_INCLUDE_DIR ${BINARY_DIR}/build/include CACHE INTERNAL "")

add_library(FFmpeg_external SHARED IMPORTED)
set(FFmpeg_LIBRARIES
  ${FFmpeg_LIBRARY_DIR}/libavutil.a
  ${FFmpeg_LIBRARY_DIR}/libavformat.a
  ${FFmpeg_LIBRARY_DIR}/libavcodec.a
  ${FFmpeg_LIBRARY_DIR}/libavdevice.a
  ${FFmpeg_LIBRARY_DIR}/libavfilter.a
  ${FFmpeg_LIBRARY_DIR}/libpostproc.a
  ${FFmpeg_LIBRARY_DIR}/libswresample.a
  ${FFmpeg_LIBRARY_DIR}/libswscale.a
  CACHE INTERNAL "" 
)

message(STATUS "FFmpeg_DIR: ${FFmpeg_LIBRARY_DIR}")
