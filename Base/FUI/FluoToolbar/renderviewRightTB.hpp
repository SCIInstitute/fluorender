#ifndef RENDERVIEW_RIGHT_TOOLBAR_HPP
#define RENDERVIEW_RIGHT_TOOLBAR_HPP

#include "genToolbarObjects.hpp"

class RightToolbar : public QToolBar
{
  Q_OBJECT

  public:
    RightToolbar();


  private:
    void initializeActions();
    void setActionProperties();
    void addWidgets();
    void setToolbarProperties();

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
