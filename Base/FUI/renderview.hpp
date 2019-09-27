#ifndef RENDERVIEW_HPP
#define RENDERVIEW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QDockWidget>
#include <QOpenGLWidget>
#include <QToolBar>
#include <QFrame>
#include <QPalette>
#include <QString>
#include <QStringList>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QPushButton>
#include <QComboBox>
#include <QPixmap>
#include <QIcon>
#include <QColorDialog>

#include <memory>
#include <iostream>

#include "fluoglwidget.hpp"
#include "renderviewLeftTB.hpp"
#include "renderviewRightTB.hpp"
#include "renderviewBottomTB.hpp"
#include "renderviewTopTB.hpp"

class RenderView : public QMainWindow
{
    Q_OBJECT

  public:
    RenderView(QWidget *parent = nullptr, bool hasFeatures = true, int renderNumber = 0);
    RenderView(bool hasFeatures, int renderNumber) : RenderView(nullptr,hasFeatures,renderNumber) {}

    bool getMainWindowStatus();

  private:
    bool isMainWindow = false;

};

#endif // RENDERVIEW_HPP
