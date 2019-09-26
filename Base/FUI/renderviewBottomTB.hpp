#ifndef RENDERVIEW_BOTTOM_TOOLBAR_HPP
#define RENDERVIEW_BOTTOM_TOOLBAR_HPP

#include "genToolbarObjects.hpp"

class BottomToolbar : public QToolBar
{
  public:
    BottomToolbar();
    void initializeActions();
    void initializeLabels();
    void initializeSliders();
    void initializeSpinBoxes();
    void addWidgetsToLayout();
    void addWidgetsToToolbar();
    void setToolbarProperties();

  private:
    std::unique_ptr<QLabel> xLabel;
    std::unique_ptr<QLabel> yLabel;
    std::unique_ptr<QLabel> zLabel;
    std::unique_ptr<QSlider> xSlider;
    std::unique_ptr<QSlider> ySlider;
    std::unique_ptr<QSlider> zSlider;
    std::unique_ptr<QDoubleSpinBox> xSpinBox;
    std::unique_ptr<QDoubleSpinBox> ySpinBox;
    std::unique_ptr<QDoubleSpinBox> zSpinBox;
    std::unique_ptr<QComboBox> labelDropDown;
    std::unique_ptr<QAction> angleButton;
    std::unique_ptr<QAction> globeButton;
    std::unique_ptr<QAction> resetButton;
    std::unique_ptr<QWidget> sliderWidgets;
    std::unique_ptr<QHBoxLayout> sliderLayout;

    QStringList labels = {"NA","+X","-X","+Y","-Y","+Z","-Z"};
};

#endif
