#include "renderviewLeftTB.hpp"

LeftToolbar::LeftToolbar()
{
  sliderSpinBoxWidget = std::make_unique<QWidget>();
  sliderSpinBoxLayout = std::make_unique<QVBoxLayout>();

  slider = genSlider(Qt::Vertical,0,1);
  spinBox = genSpinBox<QDoubleSpinBox,double>(0.0,1.0);
  fullCircle = genActionButton(":/fullCircle.svg");
  resetButton = genActionButton(":/reset.svg");

  sliderSpinBoxLayout->setAlignment(Qt::AlignHCenter);
  sliderSpinBoxLayout->addWidget(slider.release());
  sliderSpinBoxLayout->addWidget(spinBox.release());

  sliderSpinBoxWidget->setLayout(sliderSpinBoxLayout.release());

  this->addAction(fullCircle.release());
  this->addWidget(sliderSpinBoxWidget.release());
  this->addSeparator();
  this->addAction(resetButton.release());

  this->setMovable(false);
  this->setOrientation(Qt::Vertical);
  this->setFixedWidth(35);
}
