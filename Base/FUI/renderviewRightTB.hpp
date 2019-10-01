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
    QLabel* zoomLabel;
    QAction* lightBulb;
    QSlider* slider;
    QSpinBox* spinBox;
    QAction* tvButton;
    QAction* resetButton;

    QWidget* sliderSpinBoxWidget;
    QVBoxLayout* sliderSpinBoxLayout;
};

#endif
