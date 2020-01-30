#include "propertiesPanel.hpp"
#include <Global/Global.hpp>

PropertiesPanel::PropertiesPanel()
{
  myLayout->addWidget(tabWidget);
  this->setLayout(myLayout);
  m_agent = fluo::Global::instance().getAgentFactory().getOrAddVolumePropAgent("VolumePropPanel",*this);
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

void PropertiesPanel::setPropSatValue(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setSaturationVal(newVal);
}

void PropertiesPanel::setPropLowThreshValue(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLowThreshValue(newVal);
}

void PropertiesPanel::setPropHighThreshValue(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setHighThreshValue(newVal);
}

void PropertiesPanel::setPropLuminValue(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setLuminanceVal(newVal);
}

void PropertiesPanel::setPropShadowEnabled(bool status)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setShadowEnabled(status);
}

void PropertiesPanel::setPropAlphaValue(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setAlphaVal(newVal);

}

void PropertiesPanel::setPropAlphaEnabled(bool status)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setAlphaEnabled(status);
}

void PropertiesPanel::setPropShaderEnabled(bool status)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setShaderEnabled(status);
}

void PropertiesPanel::setPropLowColorModeValue(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setColorMapLowVal(newVal);
}

void PropertiesPanel::setPropHighColorModeValue(int newVal)
{
  VolumePropertiesOptions* temp = getPropertiesOptions();

  temp->setColorMapHighVal(newVal);
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

  connect(newVolumePropOpt,&VolumePropertiesOptions::sendGammaValue,this,&PropertiesPanel::onGammaReceived);

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
