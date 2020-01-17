#ifndef FLUO_SLIDER_SPINBOX_CONTROLLER_HPP
#define FLUO_SLIDER_SPINBOX_CONTROLLER_HPP

#include "controller.hpp"
#include "fluoSlider.hpp"
#include "fluoSpinbox.hpp"

#include <QObject>

class FluoSliderSpinController : QObject
{

  Q_OBJECT

  public:
    FluoSliderSpinController(FluoSlider* slider, FluoSpinbox* spinBox);

  private:
    Controller<FluoSlider,FluoSpinbox> *controller;
    FluoSlider *contSlider;
    FluoSpinbox *contSpinBox;

  private slots:
    void on_slider_valueChanged(int value);

    void on_spinBox_editingFinished();
};

#endif
