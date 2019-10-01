#include "renderviewLeftTB.hpp"

LeftToolbar::LeftToolbar()
{
  setToolbarProperties();
  initializeActions();
  disableSliderSpin();
  sliderSpinBoxLayout->setAlignment(Qt::AlignHCenter);
  sliderSpinBoxLayout->addWidget(slider);

  sliderSpinBoxWidget->setLayout(sliderSpinBoxLayout);

  connect(toggleAction,SIGNAL(triggered()),this,SLOT(on_toggleButton_clicked()));

  addWidgets();

}

void LeftToolbar::rotateImages()
{
  if(imageID == 1)
  {
    imageID = 0;
    toggleAction->setIcon(QIcon(images[imageID]));
    disableSliderSpin();
  }
  else
  {
    imageID += 1;
    toggleAction->setIcon(QIcon(images[imageID]));
    enableSliderSpin();
  }
}

void LeftToolbar::initializeActions()
{
  sliderSpinBoxWidget = new QWidget();
  sliderSpinBoxLayout = new QVBoxLayout();

  slider = genSlider(Qt::Vertical,0,1);
  spinBox = genSpinBox<QDoubleSpinBox,double>(0.0,1.0);
  toggleAction = genActionButton(":/fullCircle.svg");
  resetButton = genActionButton(":/reset.svg");
}

void LeftToolbar::disableSliderSpin()
{
  slider->setEnabled(false);
  spinBox->setEnabled(false);
}

void LeftToolbar::enableSliderSpin()
{
  slider->setEnabled(true);
  spinBox->setEnabled(true);
}

void LeftToolbar::addWidgets()
{
  this->addAction(toggleAction);
  this->addWidget(sliderSpinBoxWidget);
  this->addWidget(spinBox);
  this->addSeparator();
  this->addAction(resetButton);
}

void LeftToolbar::setToolbarProperties()
{
  this->setMovable(false);
  this->setStyleSheet("QToolBar {background: rgb(222,225,232)}");
  this->setOrientation(Qt::Vertical);
  this->setFixedWidth(35);
}
