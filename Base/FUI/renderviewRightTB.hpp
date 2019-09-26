#ifndef RENDERVIEW_RIGHT_TOOLBAR_HPP
#define RENDERVIEW_RIGHT_TOOLBAR_HPP

#include "genToolbarObjects.hpp"

class RightToolbar : public QToolBar
{
  public:
    RightToolbar();
    void initializeActions();
    void addWidgets();
    void setToolbarProperties();

  private:
    std::unique_ptr<QLabel> zoomLabel;
    std::unique_ptr<QAction> lightBulb;
    std::unique_ptr<QSlider> slider;
    std::unique_ptr<QSpinBox> spinBox;
    std::unique_ptr<QAction> tvButton;
    std::unique_ptr<QAction> resetButton;

    std::unique_ptr<QWidget> sliderSpinBoxWidget;
    std::unique_ptr<QVBoxLayout> sliderSpinBoxLayout;
};

#endif
