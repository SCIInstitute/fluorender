project(FLIVR)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


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

add_library(FLIVR STATIC ${HEADER_FILES} ${SOURCES}) 
add_dependencies(FLIVR FLObject)
target_link_libraries(FLIVR FLObject)
