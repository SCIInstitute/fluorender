#ifndef RENDERVIEW_LEFT_TOOLBAR_HPP
#define RENDERVIEW_LEFT_TOOLBAR_HPP

#include "genToolbarObjects.hpp"

class LeftToolbar : public QToolBar
{
  public:
    LeftToolbar();
    void initializeActions();
    void addWidgets();
    void setToolbarProperties();

  private:
    std::unique_ptr<QAction> fullCircle;
    std::unique_ptr<QAction> resetButton;
    std::unique_ptr<QSlider> slider;
    std::unique_ptr<QDoubleSpinBox> spinBox;
    std::unique_ptr<QWidget> sliderSpinBoxWidget;
    std::unique_ptr<QVBoxLayout> sliderSpinBoxLayout;
};

#endif
