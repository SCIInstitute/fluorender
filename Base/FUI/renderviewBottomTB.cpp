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

  connect(sliderButton,SIGNAL(triggered()),this,SLOT(on_sliderButton_clicked()));
}

void BottomToolbar::rotateImages()
{
  if(imageID == 1)
  {
    imageID = 0;
    sliderButton->setIcon(QIcon(images[imageID]));
  }
  else
  {
    imageID += 1;
    sliderButton->setIcon(QIcon(images[imageID]));
  }
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
  sliderButton = genActionButton(":/globe.svg");

  angleButton->setCheckable(true);
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
  this->addAction(sliderButton);
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
