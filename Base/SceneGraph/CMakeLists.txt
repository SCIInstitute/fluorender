project(SceneGraph)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${FLObject_INCLUDES} ${FLIVR_INCLUDE_DIRS})

set(HEADER_FILES Annotations/Annotations.hpp
                 Data/MeshData/MeshData.hpp
                 Data/VolumeData/VolumeData.hpp
                 DrawVolumeVisitor/DrawVolumeVisitor.hpp
                 Factories/Annotations/AnnotationFactory.hpp
                 Factories/Mesh/MeshFactory.hpp
                 Factories/Volume/VolumeFactory.hpp
                 Groups/Group.hpp
                 Groups/Mesh/MeshGroup.hpp
                 Groups/Volume/VolumeGroup.hpp
                 Node/Node.hpp
                 NodeVisitor/NodeVisitor.hpp
                 RenderView/RenderView.hpp
                 Root/Root.hpp
                 Utilities/DecycleVisitor.hpp
                 Utilities/InfoVisitor.hpp
                 Utilities/Layer.hpp
                 Utilities/PopVolumeVisitor.hpp
                 Utilities/SearchVisitor.hpp
                 Utilities/ValueUpdateVisitor.hpp)

set(SOURCES Annotations/Annotations.cpp
            Data/MeshData/MeshData.cpp
            Data/VolumeData/VolumeData.cpp
            DrawVolumeVisitor/DrawVolumeVisitor.cpp
            Factories/Annotations/AnnotationFactory.cpp
            Factories/Mesh/MeshFactory.cpp
            Factories/Volume/VolumeFactory.cpp
            Groups/Group.cpp
            Groups/Mesh/MeshGroup.cpp
            Groups/Volume/VolumeGroup.cpp
            Node/Node.cpp
            NodeVisitor/NodeVisitor.cpp
            RenderView/RenderView.cpp
            Root/Root.cpp)


set(SceneGraph_INCLUDES ${HEADER_FILES} CACHE INTERNAL "")
set(SceneGraph_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")

add_library(SceneGraph STATIC ${HEADER_FILES} ${SOURCES}) 

target_include_directories(SceneGraph PUBLIC Annotations
    Data
    DrawVolumeVisitor
    Factories
    Groups
    Node
    NodeVisitor
    RenderView
    Root
    Utilities
    )

target_link_libraries(SceneGraph FLObject FLIVR)

install(TARGETS SceneGraph
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#target_link_libraries(FLObject Boost)
