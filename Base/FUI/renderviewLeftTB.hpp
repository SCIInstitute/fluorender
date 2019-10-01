#ifndef RENDERVIEW_LEFT_TOOLBAR_HPP
#define RENDERVIEW_LEFT_TOOLBAR_HPP

#include "genToolbarObjects.hpp"
#include <vector>

class LeftToolbar : public QToolBar
{
  Q_OBJECT

  public slots:
    void on_toggleButton_clicked() { rotateImages(); }

  public:
    LeftToolbar();

  private:
    void initializeActions();
    void disableSliderSpin();
    void enableSliderSpin();
    void addWidgets();
    void setToolbarProperties();
    void rotateImages();

    QAction* toggleAction;
    QAction* resetButton;
    QSlider *slider;
    QDoubleSpinBox* spinBox;
    QWidget* sliderSpinBoxWidget;
    QVBoxLayout* sliderSpinBoxLayout;

    int imageID = 0;
    const std::vector<QString> images = {":/fullCircle.svg",":/halfCircle.svg"};
};

#endif
