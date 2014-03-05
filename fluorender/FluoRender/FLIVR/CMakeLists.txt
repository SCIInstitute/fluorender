#This is an initial CMake configuration file for FluoRender

include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Biff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Air)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Hest)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/teem/Nrrd)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/glew)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/LibTiff)
include_directories(${FluoRender_SOURCE_DIR}/fluorender/FluoRender)

add_library( LibFLIVR 	BBox.cpp
			Color.cpp
			glm.cpp
			ImgShader.cpp
			MeshRenderer.cpp
			MshShader.cpp
			MultiVolumeRenderer.cpp
			PaintShader.cpp
			Plane.cpp
			Point.cpp
			Ray.cpp
			SegShader.cpp
			ShaderProgram.cpp
			Texture.cpp
			TextureBrick.cpp
			TextureRenderer.cpp
			Transform.cpp
			Vector.cpp
			VolCalShader.cpp
			VolShader.cpp
			VolumeRenderer.cpp)
