QT += gui widgets opengl openglwidgets uiplugin

requires(qtConfig(filedialog))

FLUORENDER_DIR = "Src/FluoRender"
UI_DIR = "$${FLUORENDER_DIR}/UI"

HEADERS       = \
  $${UI_DIR}/MainWindow/QvisDataManager.h \
  $${UI_DIR}/MainWindow/QvisMainWindow.h \
  $${UI_DIR}/MainWindow/QvisMessageDialog.h \
  $${UI_DIR}/MainWindow/QvisViewWindow.h \
  $${UI_DIR}/Preferences/CapturePreferencesAttributes.h \
  $${UI_DIR}/Preferences/MoviePreferencesAttributes.h \
  $${UI_DIR}/Preferences/QvisCapturePreferencesDialog.h \
  $${UI_DIR}/Preferences/QvisMoviePreferencesDialog.h \
  $${UI_DIR}/Preferences/QvisPreferencesDialog.h \
  $${UI_DIR}/Properties/QvisMeshPropertiesDialog.h \
  $${UI_DIR}/Properties/QvisOutputImagePropertiesDialog.h \
  $${UI_DIR}/Properties/QvisViewPropertiesDialog.h \
  $${UI_DIR}/Properties/QvisVolumePropertiesDialog.h \
  $${UI_DIR}/Tools/QvisCalculationDialog.h \
  $${UI_DIR}/Tools/QvisClipDialog.h \
  $${UI_DIR}/Tools/QvisColocalizationDialog.h \
  $${UI_DIR}/Tools/QvisComponentAnalyzerDialog.h \
  $${UI_DIR}/Tools/QvisConvertDialog.h \
  $${UI_DIR}/Tools/QvisExportDialog.h \
  $${UI_DIR}/Tools/QvisMeasurementDialog.h \
  $${UI_DIR}/Tools/QvisMeshTransformDialog.h \
  $${UI_DIR}/Tools/QvisNoiseReductionDialog.h \
  $${UI_DIR}/Tools/QvisOpenCLKernelEditorDialog.h \
  $${UI_DIR}/Tools/QvisPaintBrushDialog.h \
  $${UI_DIR}/Tools/QvisTrackingDialog.h \
  $${UI_DIR}/Tools/QvisVolumeSizeDialog.h \
  $${UI_DIR}/ViewWindow/QvisViewWindowDialog.h \
  $${UI_DIR}/Widgets/AttributesBase.h \
  $${UI_DIR}/Widgets/QvisDialogBase.h \
  $${UI_DIR}/Widgets/QvisOpenGLWidget.h \
  $${UI_DIR}/Widgets/QvisRangeSlider.h \
  $${UI_DIR}/Widgets/QvisDragDropToolBar.h \
  $${UI_DIR}/Widgets/QvisDragDropWidget.h \
  $${UI_DIR}/Widgets/QvisTreeWidget.h \
  $${UI_DIR}/Widgets/QvisVCRWidget.h

SOURCES = $${FLUORENDER_DIR}/Main/Main.cpp \
  $${UI_DIR}/MainWindow/QvisDataManager.cpp \
  $${UI_DIR}/MainWindow/QvisMainWindow.cpp \
  $${UI_DIR}/MainWindow/QvisMessageDialog.cpp \
  $${UI_DIR}/MainWindow/QvisViewWindow.cpp \
  $${UI_DIR}/Preferences/CapturePreferencesAttributes.cpp \
  $${UI_DIR}/Preferences/MoviePreferencesAttributes.cpp \
  $${UI_DIR}/Preferences/QvisCapturePreferencesDialog.cpp \
  $${UI_DIR}/Preferences/QvisMoviePreferencesDialog.cpp \
  $${UI_DIR}/Preferences/QvisPreferencesDialog.cpp \
  $${UI_DIR}/Properties/QvisMeshPropertiesDialog.cpp \
  $${UI_DIR}/Properties/QvisOutputImagePropertiesDialog.cpp \
  $${UI_DIR}/Properties/QvisViewPropertiesDialog.cpp \
  $${UI_DIR}/Properties/QvisVolumePropertiesDialog.cpp \
  $${UI_DIR}/Tools/QvisCalculationDialog.cpp \
  $${UI_DIR}/Tools/QvisClipDialog.cpp \
  $${UI_DIR}/Tools/QvisColocalizationDialog.cpp \
  $${UI_DIR}/Tools/QvisComponentAnalyzerDialog.cpp \
  $${UI_DIR}/Tools/QvisConvertDialog.cpp \
  $${UI_DIR}/Tools/QvisExportDialog.cpp \
  $${UI_DIR}/Tools/QvisMeasurementDialog.cpp \
  $${UI_DIR}/Tools/QvisMeshTransformDialog.cpp \
  $${UI_DIR}/Tools/QvisNoiseReductionDialog.cpp \
  $${UI_DIR}/Tools/QvisOpenCLKernelEditorDialog.cpp \
  $${UI_DIR}/Tools/QvisPaintBrushDialog.cpp \
  $${UI_DIR}/Tools/QvisTrackingDialog.cpp \
  $${UI_DIR}/Tools/QvisVolumeSizeDialog.cpp \
  $${UI_DIR}/ViewWindow/QvisViewWindowDialog.cpp \
  $${UI_DIR}/Widgets/AttributesBase.cpp \
  $${UI_DIR}/Widgets/QvisDialogBase.cpp \
  $${UI_DIR}/Widgets/QvisOpenGLWidget.cpp \
  $${UI_DIR}/Widgets/QvisRangeSlider.cpp \
  $${UI_DIR}/Widgets/QvisDragDropToolBar.cpp \
  $${UI_DIR}/Widgets/QvisDragDropWidget.cpp \
  $${UI_DIR}/Widgets/QvisTreeWidget.cpp \
  $${UI_DIR}/Widgets/QvisVCRWidget.cpp

FORMS += \
  $${UI_DIR}/MainWindow/QvisDataManager.ui \
  $${UI_DIR}/MainWindow/QvisMainWindow.ui \
  $${UI_DIR}/MainWindow/QvisMessageDialog.ui \
  $${UI_DIR}/MainWindow/QvisViewWindow.ui \
  $${UI_DIR}/Preferences/QvisCapturePreferencesDialog.ui \
  $${UI_DIR}/Preferences/QvisMoviePreferencesDialog.ui \
  $${UI_DIR}/Preferences/QvisPreferencesDialog.ui \
  $${UI_DIR}/Properties/QvisMeshPropertiesDialog.ui \
  $${UI_DIR}/Properties/QvisOutputImagePropertiesDialog.ui \
  $${UI_DIR}/Properties/QvisViewPropertiesDialog.ui \
  $${UI_DIR}/Properties/QvisVolumePropertiesDialog.ui \
  $${UI_DIR}/Tools/QvisCalculationDialog.ui \
  $${UI_DIR}/Tools/QvisClipDialog.ui \
  $${UI_DIR}/Tools/QvisColocalizationDialog.ui \
  $${UI_DIR}/Tools/QvisComponentAnalyzerDialog.ui \
  $${UI_DIR}/Tools/QvisConvertDialog.ui \
  $${UI_DIR}/Tools/QvisExportDialog.ui \
  $${UI_DIR}/Tools/QvisMeasurementDialog.ui \
  $${UI_DIR}/Tools/QvisMeshTransformDialog.ui \
  $${UI_DIR}/Tools/QvisNoiseReductionDialog.ui \
  $${UI_DIR}/Tools/QvisOpenCLKernelEditorDialog.ui \
  $${UI_DIR}/Tools/QvisPaintBrushDialog.ui \
  $${UI_DIR}/Tools/QvisTrackingDialog.ui \
  $${UI_DIR}/Tools/QvisVolumeSizeDialog.ui \
  $${UI_DIR}/ViewWindow/QvisViewWindowDialog.ui \
  $${UI_DIR}/Widgets/QvisDragDropWidget.ui \
  $${UI_DIR}/Widgets/QvisVCRWidget.ui

RESOURCES = FluoRender.qrc

ICON = $${PWD}/$${UI_DIR}/icons/FluoRender_icon.icns

# install
target.path = build/FluoRender

INSTALLS += target

INCLUDEPATH += \
 ../build/$${FLUORENDER_DIR}/include \
 $${UI_DIR} \
 $${UI_DIR}/MainWindow \
 $${UI_DIR}/Preferences \
 $${UI_DIR}/Properties \
 $${UI_DIR}/Tools \
 $${UI_DIR}/ViewWindow \
 $${UI_DIR}/Widgets

DISTFILES += \
    ads.pri

CONFIG += c++17

macx: {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
}

include(ads.pri)
