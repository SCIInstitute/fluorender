project(FLIVR)

include_directories(${OpenCL_INCLUDE_DIRS})
include_directories(${FLObject_INCLUDES})
include_directories(${freetype_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Base/FLObject)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${Teem_INCLUDE_DIR})

message("FLIVR OpenCL: ${OpenCL_INCLUDE_DIRS}")

if(APPLE)
  include_directories(${Boost_INCLUDE_DIR})
endif()

set(HEADER_FILES BBox.h
        Color.h
				Framebuffer.h
        glm.h
        ImgShader.h
        KernelProgram.h
        MeshRenderer.h
        MshShader.h
        MultiVolumeRenderer.h
        Plane.h
        Point.h
        Quaternion.h
				Ray.h
				SegShader.h
				ShaderProgram.h
				TextRenderer.h
				Texture.h
				TextureBrick.h
				TextureRenderer.h
				Transform.h
				Utils.h
				Vector.h
				VertexArray.h
				VolCalShader.h
				VolKernel.h
				VolShader.h
				VolShaderCode.h
				VolumeRenderer.h)

set(SOURCES BBox.cpp
            Color.cpp
            Framebuffer.cpp
            glm.cpp
            ImgShader.cpp
            KernelProgram.cpp
            MeshRenderer.cpp
            MshShader.cpp
            MultiVolumeRenderer.cpp
            Plane.cpp
            Point.cpp
            Ray.cpp
			SegShader.cpp
			ShaderProgram.cpp
			TextRenderer.cpp
			Texture.cpp
			TextureBrick.cpp
			TextureRenderer.cpp
            Transform.cpp
			Vector.cpp
			VertexArray.cpp
			VolCalShader.cpp
			VolKernel.cpp
			VolShader.cpp
			VolumeRenderer.cpp)


set(FLIVR_INCLUDE_DIR ${HEADER_FILES} CACHE INTERNAL "")
set(FLIVR_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")

#link_directories(${FLObject_LIBRARY_DIR} ${glew_LIBRARY_DIR} ${Teem_dir}/bin)
add_library(FLIVR STATIC ${HEADER_FILES} ${SOURCES}) 

if(APPLE)
  target_link_libraries(FLIVR FLObject ${OpenCL_LIBRARIES} ${OPENGL_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY})
  add_dependencies(FLIVR FLObject freetype_external_download glew_external_download Teem_external_download Boost_external_download)
else()
  target_link_libraries(FLIVR FLObject ${OpenCL_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${teem_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY})
  add_dependencies(FLIVR FLObject freetype_external_download glew_external_download Teem_external_download)
endif()
