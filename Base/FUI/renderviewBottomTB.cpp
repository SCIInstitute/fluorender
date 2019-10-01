#include "renderviewBottomTB.hpp"

BottomToolbar::BottomToolbar()
{
  setToolbarProperties();
  initializeActions();

  sliderLayout = new QHBoxLayout();
  sliderWidgets = new QWidget();

  addWidgetsToLayout();

  sliderWidgets->setLayout(sliderLayout);

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
  sliderLayout->addWidget(xLabel);
  sliderLayout->addWidget(xSlider);
  sliderLayout->addWidget(xSpinBox);
  sliderLayout->addWidget(yLabel);
  sliderLayout->addWidget(ySlider);
  sliderLayout->addWidget(ySpinBox);
  sliderLayout->addWidget(zLabel);
  sliderLayout->addWidget(zSlider);
  sliderLayout->addWidget(zSpinBox);
  sliderLayout->addWidget(labelDropDown);
}

void BottomToolbar::addWidgetsToToolbar()
{
  this->addAction(angleButton);
  this->addAction(globeButton);
  this->addWidget(sliderWidgets);
  this->addAction(resetButton);
}

void BottomToolbar::setToolbarProperties()
{
  this->setMovable(false);
  this->setStyleSheet("QToolBar {background: rgb(222,225,232)}");
  this->setOrientation(Qt::Horizontal);
  this->setFixedHeight(35);
}
