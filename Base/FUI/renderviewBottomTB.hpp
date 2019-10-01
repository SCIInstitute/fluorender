#ifndef RENDERVIEW_BOTTOM_TOOLBAR_HPP
#define RENDERVIEW_BOTTOM_TOOLBAR_HPP

#include "genToolbarObjects.hpp"

class BottomToolbar : public QToolBar
{
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
    QAction* globeButton;
    QAction* resetButton;
    QWidget* sliderWidgets;
    QHBoxLayout* sliderLayout;

    const QStringList labels = {"NA","+X","-X","+Y","-Y","+Z","-Z"};
};

#endif
