project(FLIVR)

include_directories(${OpenCL_INCLUDE_DIRS})
include_directories(${FLObject_INCLUDES})
include_directories(${freetype_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Base/FLObject)
include_directories(${OPENGL_INCLUDE_DIR})

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

link_directories(${FLObject_LIBRARY_DIR})
add_library(FLIVR STATIC ${HEADER_FILES} ${SOURCES}) 
target_link_libraries(FLIVR FLObject ${OpenCL_LIBRARIES} ${OPENGL_LIBRARIES})
add_dependencies(FLIVR FLObject freetype_external_download)
