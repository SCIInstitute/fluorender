# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = Glew

TEMPLATE = lib
CONFIG += staticlib c++17

HEADERS = \
   $$PWD/GL/glew.h \
   $$PWD/GL/glxew.h \
   $$PWD/GL/mywgl.h \
   $$PWD/GL/wglew.h

SOURCES = \
   $$PWD/glew.c

INCLUDEPATH = \
    $$PWD/GL

#DEFINES = 

