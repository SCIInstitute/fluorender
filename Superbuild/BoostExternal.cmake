#---------------------------------------------------------------------------
# Get and build boost


SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
set( Boost_Bootstrap_Command )

if( UNIX )
  set( Boost_url "http://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz")
  set( Boost_Hash "SHA256=882b48708d211a5f48e60b0124cf5863c1534cd544ecd0664bb534a4b5d506e9")
  set( Boost_Bootstrap_Command ./bootstrap.sh )
  set( Boost_b2_Command ./b2 )
elseif( WIN32 )
    set( Boost_url "http://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.zip")
    set( Boost_Hash "SHA256=48f379b2e90dd1084429aae87d6bdbde9670139fa7569ee856c8c86dd366039d")
    set( Boost_Bootstrap_Command bootstrap.bat )
    set( Boost_b2_Command b2.exe )
endif()

if(WIN32)
  ExternalProject_Add(Boost_external_download
    URL ${Boost_url}
    URL_HASH ${Boost_Hash}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ${Boost_Bootstrap_Command} ${BoostToolset}
    BUILD_COMMAND  ${Boost_b2_Command} install
      -toolset=${BoostToolset}
      --with-system
      --with-chrono
      --with-filesystem
      --disable-icu
      --prefix=${CMAKE_BINARY_DIR}/Boost
      --threading=single,multi
      --link=static
      --variant=release
      -j8
    INSTALL_COMMAND ""
    INSTALL_DIR ""
  )
else()
  ExternalProject_Add(Boost_external_download
    URL ${Boost_url}
    URL_HASH ${Boost_Hash}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ${Boost_Bootstrap_Command}
    BUILD_COMMAND  ${Boost_b2_Command} install
      --with-system
      --with-chrono
      --with-filesystem
      --disable-icu
      --prefix=${CMAKE_BINARY_DIR}/Boost
      --threading=single,multi
      --link=static
      --variant=release
      -j8
    INSTALL_COMMAND ""
    INSTALL_DIR ""
  )
endif()

#CACHE PATH "" seems to write the path to a file that I can set 
#library paths to. 

set(Boost_LIBRARY_DIR ${CMAKE_BINARY_DIR}/Boost/lib CACHE INTERNAL "")

if(WIN32)
  set(Boost_INCLUDE_DIR ${CMAKE_BINARY_DIR}/Boost/include/boost-1_70 CACHE INTERNAL "")
  set(BOOST_ROOT ${CMAKE_BINARY_DIR}/Boost CACHE INTERNAL "")
else()
  set(Boost_INCLUDE_DIR ${CMAKE_BINARY_DIR}/Boost/include CACHE INTERNAL "")
endif()

ExternalProject_Get_Property(Boost_external_download BINARY_DIR)
SET(Boost_DIR ${BINARY_DIR} CACHE INTERNAL "")

add_library(Boost_external STATIC IMPORTED)

message(STATUS "Boost_DIR: ${Boost_DIR}")
