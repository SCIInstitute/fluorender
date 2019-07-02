# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = FLIVR

TEMPLATE += lib

CONFIG += staticlib c++17

HEADERS = \
   $$PWD/BBox.h \
   $$PWD/Color.h \
   $$PWD/Framebuffer.h \
   $$PWD/glm.h \
   $$PWD/ImgShader.h \
   $$PWD/KernelProgram.h \
   $$PWD/MeshRenderer.h \
   $$PWD/MshShader.h \
   $$PWD/MultiVolumeRenderer.h \
   $$PWD/Plane.h \
   $$PWD/Point.h \
   $$PWD/Quaternion.h \
   $$PWD/Ray.h \
   $$PWD/SegShader.h \
   $$PWD/ShaderProgram.h \
   $$PWD/TextRenderer.h \
   $$PWD/Texture.h \
   $$PWD/TextureBrick.h \
   $$PWD/TextureRenderer.h \
   $$PWD/Transform.h \
   $$PWD/Utils.h \
   $$PWD/Vector.h \
   $$PWD/VertexArray.h \
   $$PWD/VolCalShader.h \
   $$PWD/VolKernel.h \
   $$PWD/VolShader.h \
   $$PWD/VolShaderCode.h \
   $$PWD/VolumeRenderer.h

SOURCES = \
   $$PWD/BBox.cpp \
   $$PWD/Color.cpp \
   $$PWD/Framebuffer.cpp \
   $$PWD/glm.cpp \
   $$PWD/ImgShader.cpp \
   $$PWD/KernelProgram.cpp \
   $$PWD/MeshRenderer.cpp \
   $$PWD/MshShader.cpp \
   $$PWD/MultiVolumeRenderer.cpp \
   $$PWD/Plane.cpp \
   $$PWD/Point.cpp \
   $$PWD/Ray.cpp \
   $$PWD/SegShader.cpp \
   $$PWD/ShaderProgram.cpp \
   $$PWD/TextRenderer.cpp \
   $$PWD/Texture.cpp \
   $$PWD/TextureBrick.cpp \
   $$PWD/TextureRenderer.cpp \
   $$PWD/Transform.cpp \
   $$PWD/Vector.cpp \
   $$PWD/VertexArray.cpp \
   $$PWD/VolCalShader.cpp \
   $$PWD/VolKernel.cpp \
   $$PWD/VolShader.cpp \
   $$PWD/VolumeRenderer.cpp

INCLUDEPATH = \
    $$PWD/

#DEFINES = 


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../Base/FLObject/release/ -lFLObject
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../Base/FLObject/debug/ -lFLObject
else:unix: LIBS += -L$$OUT_PWD/../../Base/FLObject/ -lFLObject

INCLUDEPATH += $$PWD/../../Base/FLObject
DEPENDPATH += $$PWD/../../Base/FLObject

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../Base/FLObject/release/libFLObject.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../Base/FLObject/debug/libFLObject.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../Base/FLObject/release/FLObject.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../Base/FLObject/debug/FLObject.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../Base/FLObject/libFLObject.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Glew/release/ -lGlew
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Glew/debug/ -lGlew
else:unix: LIBS += -L$$OUT_PWD/../Glew/ -lGlew

INCLUDEPATH += $$PWD/../Glew
DEPENDPATH += $$PWD/../Glew

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Glew/release/libGlew.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Glew/debug/libGlew.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Glew/release/Glew.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Glew/debug/Glew.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Glew/libGlew.a
