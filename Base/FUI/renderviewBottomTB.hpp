#ifndef RENDERVIEW_BOTTOM_TOOLBAR_HPP
#define RENDERVIEW_BOTTOM_TOOLBAR_HPP

#include "genToolbarObjects.hpp"
#include <vector>

class BottomToolbar : public QToolBar
{
  Q_OBJECT

  public slots:
    void on_sliderButton_clicked() { rotateImages(); }

  public:
    BottomToolbar();


  private:
    void initializeActions();
    void initializeLabels();
    void initializeSliders();
    void initializeSpinBoxes();
    void addWidgetsToLayout();
    void addWidgetsToToolbar();
    void setToolbarProperties();

    void rotateImages();

    QLabel* xLabel;
    QLabel* yLabel;
    QLabel* zLabel;
    QSlider* xSlider;
    QSlider* ySlider;
    QSlider* zSlider;
    QDoubleSpinBox* xSpinBox;
    QDoubleSpinBox* ySpinBox;
    QDoubleSpinBox* zSpinBox;
    QComboBox* labelDropDown;
    QAction* angleButton;
    QAction* sliderButton;
    QAction* resetButton;
    QWidget* sliderWidgets;
    QHBoxLayout* sliderLayout;

    const QStringList labels = {"NA","+X","-X","+Y","-Y","+Z","-Z"};

    int imageID = 0;
    const std::vector<QString> images = {":/globe.svg",":/plane.svg"};
};

#endif
