#include "renderviewTopTB.hpp"

TopToolbar::TopToolbar()
{

  setToolbarProperties();
  initializeActionsAndWidgets();
  addWidgetsToLayout();
  setRenderActionGroupSettings();
  setInformationGroupSettings();
  setScaleGroupSettings();
  setLayouts();
  addActionsToToolbar();

  connect(renderViewLayersAction,SIGNAL(triggered()),this,SLOT(on_layers_clicked()));
  connect(renderViewDepthAction,SIGNAL(triggered()),this,SLOT(on_depth_clicked()));
  connect(renderViewColorsAction,SIGNAL(triggered()),this,SLOT(on_colors_clicked()));
  connect(defaultScale,SIGNAL(triggered()),this,SLOT(on_scale_clicked()));

}

void TopToolbar::checkFlags(QAction* currentFlag)
{
  if(!flagControl.empty())
  {
    flagControl[false]->setEnabled(true);
    currentFlag->setEnabled(false);
    flagControl[false] = currentFlag;
  }
  else
  {
    currentFlag->setEnabled(false);
    flagControl[false] = currentFlag;
  }
}

void TopToolbar::enableScaleGroupActions()
{
  if(imageID == 0)
  {
    scaleSpinBox->setEnabled(false);
    scaleDropDown->setEnabled(false);
  }
  else if(imageID == 1)
  {
    scaleSpinBox->setEnabled(true);
  }
  else
  {
    scaleDropDown->setEnabled(true);
  }
}

void TopToolbar::rotateImage(QAction* currentAction)
{
  if(imageID == 2)
  {
    imageID = 0;
    currentAction->setIcon(QIcon(images[imageID]));
    enableScaleGroupActions();
  }
  else
  {
    imageID += 1;
    currentAction->setIcon(QIcon(images[imageID]));
    enableScaleGroupActions();
  }
}

void TopToolbar::initializeActionsAndWidgets()
{
  initializeLayouts();
  initializeWidgets();
  initializePushButtons();
  initializeColorDiaSlider();
  initializeLabels();
  initializeActions();
  initialzeSpinBoxes();
}

void TopToolbar::initializeLayouts()
{
  perspectiveLayout = new QHBoxLayout();
  backgroundLayout = new QHBoxLayout();
  scaleLayout = new QHBoxLayout();
}

void TopToolbar::initializeWidgets()
{
  perspectiveWidget = new QWidget();
  backgroundWidget = new QWidget();
  scaleWidget = new QWidget();
}

void TopToolbar::initializePushButtons()
{
  colorDialogWidget = new QPushButton();
  captureWidget = new QPushButton();
}

void TopToolbar::initializeColorDiaSlider()
{
  colorDialog = new QColorDialog(colorDialogWidget);
  perspectiveSlider = genSlider(Qt::Horizontal,0,100);
}

void TopToolbar::initializeLabels()
{
  perspectiveLabel = genLabel("Perspective Angle:");
  backgroundLabel = genLabel("Background:");
}

void TopToolbar::initializeActions()
{
  renderViewLayersAction = genActionButton(":/ungroupLayers.svg");
  renderViewDepthAction = genActionButton(":/depth.svg");
  renderViewColorsAction = genActionButton(":/byColor.svg");
  centerAxisAction = genActionButton(":/3dPlane.svg");
  infoAction = genActionButton(":/info.svg");
  labelAction = genActionButton(":/tag.svg");
  defaultScale = genActionButton(":/default.svg");
  freeFlyAction = genActionButton(":/bird.svg");
  saveConfigsAction = genActionButton(":/saveConfigs.svg");
}

void TopToolbar::setRenderActionGroupSettings()
{
  renderViewLayersAction->setEnabled(false);
  flagControl[false] = renderViewLayersAction;
}

void TopToolbar::setInformationGroupSettings()
{
  centerAxisAction->setCheckable(true);
  infoAction->setCheckable(true);
  labelAction->setCheckable(true);
}

void TopToolbar::setScaleGroupSettings()
{
  scaleSpinBox->setEnabled(false);
  scaleDropDown->setEnabled(false);
}

void TopToolbar::initialzeSpinBoxes()
{
  scaleSpinBox = genSpinBox<QSpinBox,int>(0,999);
  perspectiveSpinBox = genSpinBox<QSpinBox,int>(0,100);
  scaleDropDown = genComboBox(scales);
}

void TopToolbar::addWidgetsToLayout()
{
  perspectiveLayout->addWidget(perspectiveLabel);
  perspectiveLayout->addWidget(perspectiveSlider);
  perspectiveLayout->addWidget(perspectiveSpinBox);

  backgroundLayout->addWidget(backgroundLabel);
  backgroundLayout->addWidget(colorDialogWidget);

  scaleLayout->addWidget(scaleSpinBox);
  scaleLayout->addWidget(scaleDropDown);

}

void TopToolbar::setLayouts()
{
  perspectiveWidget->setLayout(perspectiveLayout);
  backgroundWidget->setLayout(backgroundLayout);
  scaleWidget->setLayout(scaleLayout);
}

void TopToolbar::addActionsToToolbar()
{
  captureWidget->setIcon(QIcon(":/camera.svg"));
  captureWidget->setText(" Capture");

  this->addAction(renderViewLayersAction);
  this->addAction(renderViewDepthAction);
  this->addAction(renderViewColorsAction);
  this->addSeparator();
  this->addWidget(captureWidget);
  this->addSeparator();
  this->addAction(centerAxisAction);
  this->addAction(infoAction);
  this->addAction(labelAction);
  this->addSeparator();
  this->addAction(defaultScale);
  this->addWidget(scaleWidget);
  this->addWidget(perspectiveWidget);
  this->addAction(freeFlyAction);
  this->addSeparator();
  this->addWidget(backgroundWidget);
  this->addAction(saveConfigsAction);
}

void TopToolbar::setToolbarProperties()
{
  this->setMovable(false);
  this->setStyleSheet("QToolBar {background: rgb(222,225,232)}");
  this->setOrientation(Qt::Horizontal);
  this->setFixedHeight(35);
}
