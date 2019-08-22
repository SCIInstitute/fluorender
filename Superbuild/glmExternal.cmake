SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

set(glm_URL "https://github.com/g-truc/glm.git")
set(glm_TAG "0.9.9.5")

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++17 HAVE_FLAG_STD_CXX17)
if(HAVE_FLAG_STD_CXX17)
  message(STATUS "Enabling C++17 Support for GLM")
  set(flag "17")
else()
  check_cxx_compiler_flag(-std=c++11 HAVE_FLAG_STD_CXX11)
  if(HAVE_FLAG_STD_CXX11)
    message(STATUS "Enabling C++11 Support for GLM")
    set(flag "11")
  else()
    message(FATAL_ERROR "Error, must have at least C++11 in order to continue!")
  endif()
endif()

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(glm_external_download
  GIT_REPOSITORY ${glm_URL}
  GIT_TAG ${glm_TAG}
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS 
    -DBUILD_SHARED_LIBS:BOOL=OFF
    -DGLM_TEST_ENABLE_CXX_${flag}:BOOL=ON
)

ExternalProject_Get_Property(glm_external_download BINARY_DIR)
ExternalProject_Get_Property(glm_external_download SOURCE_DIR)

if(MSVC)
  SET(glm_LIBRARY_DIR "${BINARY_DIR}/glm;${BINARY_DIR}/glm/Debug;${BINARY_DIR}/glm/Release" CACHE INTERNAL "")
else()
  SET(glm_LIBRARY_DIR ${BINARY_DIR}/glm CACHE INTERNAL "")
endif()

SET(glm_INCLUDE_DIR ${SOURCE_DIR} CACHE INTERNAL "")

add_library(glm_external STATIC IMPORTED)

MESSAGE(STATUS "glm_LIBRARY_DIR: ${glm_LIBRARY_DIR}")
