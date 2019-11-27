#include "renderviewRightTB.hpp"

RightToolbar::RightToolbar()
{

  setToolbarProperties();
  initializeActions();
  setActionProperties();

  sliderSpinBoxLayout->setAlignment(Qt::AlignHCenter);
  sliderSpinBoxLayout->addWidget(slider);

  sliderSpinBoxWidget->setLayout(sliderSpinBoxLayout);

  addWidgets();

}

void RightToolbar::initializeActions()
{
  sliderSpinBoxWidget = new QWidget();
  sliderSpinBoxLayout = new QVBoxLayout();

  zoomLabel = genLabel("Zoom");
  lightBulb = genActionButton(":/lightOff.svg");
  slider = genSlider(Qt::Vertical,35,999);
  spinBox = genSpinBox<QSpinBox,int>(35,999);
  tvButton = genActionButton(":/tv.svg");
  resetButton = genActionButton(":/reset.svg");
}

void RightToolbar::setActionProperties()
{
  lightBulb->setCheckable(true);
  tvButton->setCheckable(true);
}

void RightToolbar::addWidgets()
{
  this->addWidget(zoomLabel);
  this->addAction(lightBulb);
  this->addWidget(sliderSpinBoxWidget);
  this->addWidget(spinBox);
  this->addSeparator();
  this->addAction(tvButton);
  this->addAction(resetButton);
}

void RightToolbar::setToolbarProperties()
{
  this->setMovable(false);
  this->setStyleSheet("QToolBar {background: rgb(222,225,232)}");
  this->setOrientation(Qt::Vertical);
  this->setFixedWidth(35);
}
