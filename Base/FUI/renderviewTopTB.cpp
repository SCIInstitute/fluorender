#include "renderviewTopTB.hpp"

TopToolbar::TopToolbar()
{

  setToolbarProperties();
  initializeActionsAndWidgets();
  addWidgetsToLayout();
  setLayouts();
  addActionsToToolbar();

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
  perspectiveLayout = std::make_unique<QHBoxLayout>();
  backgroundLayout = std::make_unique<QHBoxLayout>();
  scaleLayout = std::make_unique<QHBoxLayout>();
}

void TopToolbar::initializeWidgets()
{
  perspectiveWidget = std::make_unique<QWidget>();
  backgroundWidget = std::make_unique<QWidget>();
  scaleWidget = std::make_unique<QWidget>();
}

void TopToolbar::initializePushButtons()
{
  colorDialogWidget = std::make_unique<QPushButton>();
  captureWidget = std::make_unique<QPushButton>();
}

void TopToolbar::initializeColorDiaSlider()
{
  colorDialog = std::make_unique<QColorDialog>(colorDialogWidget.get());
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

void TopToolbar::initialzeSpinBoxes()
{
  scaleSpinBox = genSpinBox<QSpinBox,int>(0,999);
  perspectiveSpinBox = genSpinBox<QSpinBox,int>(0,100);
  scaleDropDown = genComboBox(scales);
}

void TopToolbar::addWidgetsToLayout()
{
    perspectiveLayout->addWidget(perspectiveLabel.release());
    perspectiveLayout->addWidget(perspectiveSlider.release());
    perspectiveLayout->addWidget(perspectiveSpinBox.release());

    backgroundLayout->addWidget(backgroundLabel.release());
    backgroundLayout->addWidget(colorDialogWidget.release());

    scaleLayout->addWidget(scaleSpinBox.release());
    scaleLayout->addWidget(scaleDropDown.release());

}

void TopToolbar::setLayouts()
{
    perspectiveWidget->setLayout(perspectiveLayout.release());
    backgroundWidget->setLayout(backgroundLayout.release());
    scaleWidget->setLayout(scaleLayout.release());
}

void TopToolbar::addActionsToToolbar()
{
    captureWidget->setIcon(QIcon(":/camera.svg"));
    captureWidget->setText(" Capture");

    this->addAction(renderViewLayersAction.release());
    this->addAction(renderViewDepthAction.release());
    this->addAction(renderViewColorsAction.release());
    this->addSeparator();
    this->addWidget(captureWidget.release());
    this->addSeparator();
    this->addAction(centerAxisAction.release());
    this->addAction(infoAction.release());
    this->addAction(labelAction.release());
    this->addSeparator();
    this->addAction(defaultScale.release());
    this->addWidget(scaleWidget.release());
    this->addWidget(perspectiveWidget.release());
    this->addAction(freeFlyAction.release());
    this->addSeparator();
    this->addWidget(backgroundWidget.release());
    this->addAction(saveConfigsAction.release());
}

void TopToolbar::setToolbarProperties()
{
    this->setMovable(false);
    this->setStyleSheet("QToolBar {background: rgb(222,225,232)}");
    this->setOrientation(Qt::Horizontal);
    this->setFixedHeight(35);
}
