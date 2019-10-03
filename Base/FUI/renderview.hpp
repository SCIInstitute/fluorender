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
    void callDestructor() { rightToolBar->sayHello(); }

  private:
    bool isMainWindow = false;
    LeftToolbar* leftToolBar = new LeftToolbar();
    TopToolbar* topToolBar = new TopToolbar();
    RightToolbar* rightToolBar = new RightToolbar();
    BottomToolbar* bottomToolBar = new BottomToolbar();

};

#endif // RENDERVIEW_HPP
