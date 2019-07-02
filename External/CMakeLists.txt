project(External)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(FLIVR
          Formats
          Glew
          GLM
          NV
          Pole
          teem
          Video
          Wacutils
          XInput
          Utilities)

add_subdirectory(FLIVR)
#add_subdirectory(Formats)
#add_subdirectory(Glew)
#add_subdirectory(GLM)
#add_subdirectory(NV)
#add_subdirectory(Pole)
#add_subdirectory(teem)
#add_subdirectory(Video)
#add_subdirectory(Wacutils)
#add_subdirectory(XInput)


