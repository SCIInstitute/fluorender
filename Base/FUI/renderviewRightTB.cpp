#include "renderviewRightTB.hpp"

RightToolbar::RightToolbar()
{

  setToolbarProperties();
  initializeActions();

  sliderSpinBoxLayout->setAlignment(Qt::AlignHCenter);
  sliderSpinBoxLayout->addWidget(slider.release());
  sliderSpinBoxLayout->addWidget(spinBox.release());

  sliderSpinBoxWidget->setLayout(sliderSpinBoxLayout.release());

  addWidgets();
}

void RightToolbar::initializeActions()
{
  sliderSpinBoxWidget = std::make_unique<QWidget>();
  sliderSpinBoxLayout = std::make_unique<QVBoxLayout>();

  zoomLabel = genLabel("Zoom");
  lightBulb = genActionButton(":/lightOff.svg");
  slider = genSlider(Qt::Vertical,35,999);
  spinBox = genSpinBox<QSpinBox,int>(35,999);
  tvButton = genActionButton(":/tv.svg");
  resetButton = genActionButton(":/reset.svg");
}

void RightToolbar::addWidgets()
{
  this->addWidget(zoomLabel.release());
  this->addAction(lightBulb.release());
  this->addWidget(sliderSpinBoxWidget.release());
  this->addSeparator();
  this->addAction(tvButton.release());
  this->addAction(resetButton.release());
}

void RightToolbar::setToolbarProperties()
{
  this->setMovable(false);
  this->setStyleSheet("QToolBar {background: rgb(222,225,232)}");
  this->setOrientation(Qt::Vertical);
  this->setFixedWidth(35);
}
