project(FLObject)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${SceneGraph_INCLUDES})

set(HEADER_FILES Event/Event.hpp
                 EventHandler/EventHandler.hpp
                 Object/CopyOp.hpp
                 Object/Object.hpp
                 Object/ObjectFactory.hpp
                 Observer/Observer.hpp
                 Referenced/Referenced.hpp
                 ReferencePtr/ref_ptr.hpp
                 Value/Value.hpp
                 Types/BBox.hpp
                 Types/Color.hpp
                 Types/GLfloat4.hpp
                 Types/GLint4.hpp
                 Types/Plane.hpp
                 Types/Point.hpp
                 Types/Quaternion.hpp
                 Types/Ray.hpp
                 Types/Transform.hpp
                 Types/Utils.hpp
                 Types/Vector.hpp)

set(SOURCES Event/Event.cpp
            Object/CopyOp.cpp
            Object/Object.cpp
            Object/ObjectFactory.cpp
            Observer/Observer.cpp
            Referenced/Referenced.cpp
            Value/Value.cpp
            Types/BBox.cpp
            Types/Color.cpp
            Types/Plane.cpp
            Types/Point.cpp
            Types/Ray.cpp
            Types/Transform.cpp
            Types/Vector.cpp)

set(FLObject_INCLUDES ${HEADER_FILES} CACHE INTERNAL "")
set(FLObject_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")

add_library(FLObject STATIC ${HEADER_FILES} ${SOURCES}) 

target_include_directories(FLObject PUBLIC Event
                                           EventHandler
                                           Object
                                           Observer
                                           Referenced
                                           ReferencePtr
                                           Value
                                           Types)

add_dependencies(FLObject Boost_external_download)

target_link_libraries(FLObject SceneGraph)

install(TARGETS FLObject
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
