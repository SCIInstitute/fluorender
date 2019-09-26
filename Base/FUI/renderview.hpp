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
#include <vector>

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

    std::unique_ptr<QSlider> genSlider(Qt::Orientation ori, int floor, int ceiling);
    std::unique_ptr<QLabel> genLabel(const QString &text);
    std::unique_ptr<QAction> genActionButton(const QString &imgName);
    std::unique_ptr<QComboBox> genComboBox(const QStringList &items);



    bool getMainWindowStatus();

    template<class SpinBoxType, typename V>
    std::unique_ptr<SpinBoxType> genSpinBox(V floor, V ceiling)
    {
      auto newSpinBox = std::make_unique<SpinBoxType>();
      newSpinBox->setRange(floor,ceiling);
      newSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
      return newSpinBox;
    }

  private:
    std::unique_ptr<QToolBar> genToolProp(Qt::Orientation ori);
    bool isMainWindow = false;
    QStringList labels = {"NA","+X","-X","+Y","-Y","+Z","-Z"};
    QStringList scales = {"nm","μm","mm"};
};

#endif // RENDERVIEW_HPP
