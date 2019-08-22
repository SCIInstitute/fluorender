SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

set(pole_URL "https://github.com/Sailanarmo/pole.git")
set(pole_TAG "origin/master")


# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(pole_external_download
  GIT_REPOSITORY ${pole_URL}
  GIT_TAG ${pole_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(pole_external_download BINARY_DIR)
ExternalProject_Get_Property(pole_external_download SOURCE_DIR)

if(MSVC)
  SET(pole_LIBRARY_DIR "${BINARY_DIR};${BINARY_DIR}/Debug;${BINARY_DIR}/Release" CACHE INTERNAL "")
else()
  SET(pole_LIBRARY_DIR ${BINARY_DIR} CACHE INTERNAL "")
endif()

SET(pole_INCLUDE_DIR ${SOURCE_DIR} CACHE INTERNAL "")

add_library(pole_external STATIC IMPORTED)

MESSAGE(STATUS "pole_LIBRARY_DIR: ${pole_LIBRARY_DIR}")
