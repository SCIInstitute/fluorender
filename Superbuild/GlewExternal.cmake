SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

set(Glew_url "https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.zip/download")
set(Glew_Hash "MD5=dff2939fd404d054c1036cc0409d19f1")

ExternalProject_Add(glew_external_download
  URL ${Glew_url}
  URL_HASH ${Glew_Hash}
  UPDATE_COMMAND ""
  SOURCE_SUBDIR "build/cmake"
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_C_COMPILER:PATH=${Compiler_C}
    -DCMAKE_CXX_COMPILER:PATH=${Compiler_CXX}
)

ExternalProject_Get_Property(glew_external_download BINARY_DIR)
ExternalProject_Get_Property(glew_external_download SOURCE_DIR)


if(MSVC)
  SET(glew_LIBRARY_DIR "${BINARY_DIR}/lib;${BINARY_DIR}/lib/Debug;${BINARY_DIR}/lib/Release" CACHE INTERNAL "")
else()
  SET(glew_LIBRARY_DIR ${BINARY_DIR}/lib CACHE INTERNAL "")
endif()

set(glew_INCLUDE_DIR ${SOURCE_DIR}/include CACHE INTERNAL "")

add_library(glew_external STATIC IMPORTED)

message(STATUS "glew_DIR: ${glew_LIBRARY_DIR}")
