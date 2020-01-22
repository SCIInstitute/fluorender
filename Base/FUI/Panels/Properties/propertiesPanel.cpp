#include "propertiesPanel.hpp"
#include <iostream>

PropertiesPanel::PropertiesPanel()
{
  myLayout->addWidget(tabWidget);
  this->setLayout(myLayout);
}

double PropertiesPanel::getPropOptionsMaxVal() const
{
  const int CURRTAB = tabWidget->currentIndex();
  QWidget* tempMainWidget = tabWidget->widget(CURRTAB);
  VolumePropertiesOptions* temp = tempMainWidget->findChild<VolumePropertiesOptions*>();

  //Todo, throw an exception if it comes back null
  return temp->getMaxVal();
}

void PropertiesPanel::setPropOptionsMaxVal(double newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  //Todo, throw an exception if it comes back null
  temp->setMaxVal(newVal);
}

void PropertiesPanel::setPropGammaSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setGammaSliderVal(newVal);
}

void PropertiesPanel::setPropGammaSpinboxVal(double newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setGammaSpinboxVal(newVal);
}

void PropertiesPanel::setPropExtBounSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setExtBounSliderVal(newVal);
}

void PropertiesPanel::setPropExtBounSpinboxVal(double newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setExtBounSpinBoxVal(newVal);
}

void PropertiesPanel::setPropSatSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setSatSliderVal(newVal);
}

void PropertiesPanel::setPropSatSpinboxVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setSatSpinboxVal(newVal);
}

void PropertiesPanel::setPropLowThreshSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLowThreshSliderVal(newVal);
}

void PropertiesPanel::setPropLowThreshSpinboxVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLowThreshSpinboxVal(newVal);
}

void PropertiesPanel::setPropHighThreSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setHighThreSliderVal(newVal);
}

void PropertiesPanel::setPropHighThreSpinboxVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setHighThreSpinboxVal(newVal);
}

void PropertiesPanel::setPropLuminSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLuminSliderVal(newVal);
}

void PropertiesPanel::setPropLuminSpinboxVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLuminSpinboxVal(newVal);
}

void PropertiesPanel::setPropShadowSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setShadowSliderVal(newVal);
}

void PropertiesPanel::setPropShadowSpinboxVal(double newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setShadowSpinboxVal(newVal);
}

void PropertiesPanel::setPropShadowEnabled(bool status)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setShadowEnabled(status);
}

void PropertiesPanel::setPropAlphaSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setAlphaSliderVal(newVal);
}

void PropertiesPanel::setPropAlphaSpinboxVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setAlphaSpinboxVal(newVal);
}

void PropertiesPanel::setPropAlphaEnabled(bool status)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setAlphaEnabled(status);
}

void PropertiesPanel::setPropSampSliderVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setSampleSliderVal(newVal);
}

void PropertiesPanel::setPropSampSpinboxVal(double newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setSampleSpinboxVal(newVal);
}

void PropertiesPanel::setPropLShadSlidVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLowShadeSliderVal(newVal);
}

void PropertiesPanel::setPropLShadSpinVal(double newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLowShadeSpinboxVal(newVal);
}

void PropertiesPanel::setPropHShadSlidVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setHighShadeSliderVal(newVal);
}

void PropertiesPanel::setPropHShadSpinVal(double newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setHighShadeSpinboxVal(newVal);
}

void PropertiesPanel::setPropShaderEnabled(bool status)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setShaderEnabled(status);
}

void PropertiesPanel::setPropLCMSlidVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setCMLowSliderVal(newVal);
}

void PropertiesPanel::setPropLCMSpinVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setCMLowSpinboxVal(newVal);
}

void PropertiesPanel::setPropHCMSlidVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setCMHighSliderVal(newVal);
}

void PropertiesPanel::setPropHCMSpinVal(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setCMHighSpinboxVal(newVal);
}

void PropertiesPanel::setPropCMEnabled(bool status)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setColorMapEnabled(status);
}

void PropertiesPanel::onVolumeLoaded(int renderviewID)
{
  VolumePropertiesOptions* newVolumePropOpt = new VolumePropertiesOptions();
  VolumePropertiesMisc *newVolumePropsMisc = new VolumePropertiesMisc();

  QFrame *leftFrame = genLeftFrame(newVolumePropOpt);
  QFrame *rightFrame = genRightFrame(newVolumePropsMisc);

  QWidget *mainWidget = genMainWidget(leftFrame,rightFrame);
  tabWidget->addTab(mainWidget,"Renderview: " + QString::number(renderviewID));
}

void PropertiesPanel::onMeshLoaded(int renderviewID)
{
  MeshPropertiesMaterials *newMeshPropertiesMaterial = new MeshPropertiesMaterials();
  MeshPropertiesOptions *newMeshProperties = new MeshPropertiesOptions();

  QFrame *leftFrame = genLeftFrame(newMeshPropertiesMaterial);
  QFrame *rightFrame = genRightFrame(newMeshProperties);

  QWidget *mainWidget = genMainWidget(leftFrame,rightFrame);
  tabWidget->addTab(mainWidget,"Renderview: " + QString::number(renderviewID));
}

QWidget* PropertiesPanel::genMainWidget(QFrame *left, QFrame *right)
{
  QGridLayout *newLayout = new QGridLayout();
  QWidget *newWidget = new QWidget();

  newLayout->addWidget(left,0,0);
  newLayout->addWidget(right,0,1);
  newWidget->setLayout(newLayout);

  return newWidget;
}

VolumePropertiesOptions* PropertiesPanel::getPropertiesOptions()
{
  const int CURRTAB = tabWidget->currentIndex();
  QWidget* tempMain = tabWidget->widget(CURRTAB);
  VolumePropertiesOptions* temp = tempMain->findChild<VolumePropertiesOptions*>();

  return temp;
}

VolumePropertiesMisc* PropertiesPanel::getPropertiesMisc()
{
  const int CURRTAB = tabWidget->currentIndex();
  QWidget* tempMain = tabWidget->widget(CURRTAB);
  VolumePropertiesMisc* temp = tempMain->findChild<VolumePropertiesMisc*>();

  return temp;
}
