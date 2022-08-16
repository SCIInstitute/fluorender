ADS_OUT_ROOT = /Users/allen/Projects/SRC/QtAdvDocking/build-ads-Qt_6_2_3_for_macOS-Release/
ADS_INCLUDE_DIR = /Users/allen/Projects/SRC/QtAdvDocking/Qt-Advanced-Docking-System/src

LIBS += -L$${ADS_OUT_ROOT}/lib

#QMAKE_LIBDIR += $${ADS_OUT_ROOT}/lib
#QMAKE_LFLAGS += "-Wl,-rpath,\'$${ADS_OUT_ROOT}/lib\'"

#QMAKE_RPATH += $${ADS_OUT_ROOT}/lib

#INCLUDEPATH += $${ADS_INCLUDE_DIR}/include
INCLUDEPATH += $${ADS_INCLUDE_DIR}

CONFIG(debug, debug|release) {
    win32 {
        versionAtLeast(QT_VERSION, 5.15.0) {
                LIBS += -lqtadvanceddocking
        }
        else {
                LIBS += -lqtadvanceddockingd
        }
    }
    else:mac {
        LIBS += -lqtadvanceddocking #_debug
    }
    else {
        LIBS += -lqtadvanceddocking
    }
}
else{
    LIBS += -lqtadvanceddocking
}

unix:!macx {
    LIBS += -lxcb
}
