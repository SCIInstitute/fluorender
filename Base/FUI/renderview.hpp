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
#include <QSlider>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QLayout>

#include <memory>
#include <iostream>

#include "fluoglwidget.hpp"

class RenderView : public QMainWindow
{
    Q_OBJECT

  public:
    RenderView(QWidget *parent = nullptr, bool hasFeatures = true, int renderNumber = 0);
    RenderView(bool hasFeatures, int renderNumber) : RenderView(nullptr,hasFeatures,renderNumber) {}
    void populateLeftToolBar(std::unique_ptr<QToolBar> &leftToolBar);
    void populateRightToolBar(std::unique_ptr<QToolBar> &rightToolBar);
    void populateTopToolBar(std::unique_ptr<QToolBar> &topToolBar);
    void populateBottomToolBar(std::unique_ptr<QToolBar> &bottomToolBar);

    std::unique_ptr<QSlider> createLeftSlider();

    bool getMainWindowStatus();

  private:
    std::unique_ptr<QToolBar> genToolProp(Qt::Orientation ori);
    bool isMainWindow = false;
};

#endif // RENDERVIEW_HPP
