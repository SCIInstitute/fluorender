#include "renderviewBottomTB.hpp"

BottomToolbar::BottomToolbar()
{
  setToolbarProperties();
  initializeActions();

  addWidgetsToLayout();

  sliderWidgets->setLayout(sliderLayout.release());

  addWidgetsToToolbar();
}

void BottomToolbar::initializeLabels()
{
  xLabel = genLabel("X:");
  yLabel = genLabel("Y:");
  zLabel = genLabel("Z:");
}

void BottomToolbar::initializeSliders()
{
  xSlider = genSlider(Qt::Horizontal,0,360);
  ySlider = genSlider(Qt::Horizontal,0,360);
  zSlider = genSlider(Qt::Horizontal,0,360);
}

void BottomToolbar::initializeSpinBoxes()
{
  xSpinBox = genSpinBox<QDoubleSpinBox,double>(0.0,360);
  ySpinBox = genSpinBox<QDoubleSpinBox,double>(0.0,360);
  zSpinBox = genSpinBox<QDoubleSpinBox,double>(0.0,360);
}

void BottomToolbar::initializeActions()
{
  initializeLabels();
  initializeSliders();
  initializeSpinBoxes();

  labelDropDown = genComboBox(labels);
  resetButton = genActionButton(":/reset.svg");
  angleButton = genActionButton(":/fortyFive.svg");
  globeButton = genActionButton(":/globe.svg");
}

void BottomToolbar::addWidgetsToLayout()
{
  sliderLayout->setAlignment(Qt::AlignHCenter);
  sliderLayout->addWidget(xLabel.release());
  sliderLayout->addWidget(xSlider.release());
  sliderLayout->addWidget(xSpinBox.release());
  sliderLayout->addWidget(yLabel.release());
  sliderLayout->addWidget(ySlider.release());
  sliderLayout->addWidget(ySpinBox.release());
  sliderLayout->addWidget(zLabel.release());
  sliderLayout->addWidget(zSlider.release());
  sliderLayout->addWidget(zSpinBox.release());
  sliderLayout->addWidget(labelDropDown.release());
}

void BottomToolbar::addWidgetsToToolbar()
{
  this->addAction(angleButton.release());
  this->addAction(globeButton.release());
  this->addWidget(sliderWidgets.release());
  this->addAction(resetButton.release());
}

void BottomToolbar::setToolbarProperties()
{
  this->setMovable(false);
  this->setStyleSheet("QToolBar {background: rgb(222,225,232)}");
  this->setOrientation(Qt::Vertical);
  this->setFixedWidth(35);
}
