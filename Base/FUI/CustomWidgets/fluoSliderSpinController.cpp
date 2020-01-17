#include "fluoSliderSpinController.hpp"

FluoSliderSpinController::FluoSliderSpinController(FluoSlider* slider, FluoSpinbox* spinBox)
{
  contSlider = slider;
  contSpinBox = spinBox;
  controller = new Controller<FluoSlider,FluoSpinbox>(*slider,*spinBox);
}

void FluoSliderSpinController::on_slider_valueChanged(int value)
{
  controller->setValues(value);
}

void FluoSliderSpinController::on_spinBox_editingFinished()
{
  controller->setValues(contSpinBox->value());
}
