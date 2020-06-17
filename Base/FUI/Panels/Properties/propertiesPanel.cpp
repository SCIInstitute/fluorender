#include "propertiesPanel.hpp"
#include <Global/Global.hpp>

#include <iostream>

PropertiesPanel::PropertiesPanel()
{
  myLayout->addWidget(tabWidget);
  this->setLayout(myLayout);
  //m_agent = fluo::Global::instance().getAgentFactory().getOrAddVolumePropAgent("VolumePropPanel",*this);

  //TODO: Look into if this causese memory issues in Windows.
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

void PropertiesPanel::setPropSizeLimitValue(int newVal)
{
  MeshPropertiesOptions* temp = getMeshPropertiesOptions();
  temp->setSizeLimitValue(newVal);
}

void PropertiesPanel::setPropShininessValue(int newVal)
{
  MeshPropertiesMaterials* temp = getMeshPropertiesMaterials();

  temp->setShininessValue(newVal);
}

void PropertiesPanel::onVolumeLoaded(int renderviewID)
{
  m_agent = fluo::Global::instance().getAgentFactory().getOrAddVolumePropAgent("VolumePropPanel",*this);
  VolumePropertiesOptions* newVolumePropOpt = new VolumePropertiesOptions(m_agent);
  VolumePropertiesMisc *newVolumePropsMisc = new VolumePropertiesMisc();

  makeVolumeConnections(newVolumePropOpt);

  QFrame *leftFrame = genLeftFrame(newVolumePropOpt);
  QFrame *rightFrame = genRightFrame(newVolumePropsMisc);

  QWidget *mainWidget = genMainWidget(leftFrame,rightFrame);
  tabWidget->addTab(mainWidget,"Renderview: " + QString::number(renderviewID));
}

void PropertiesPanel::onMeshLoaded(int renderviewID)
{
  MeshPropertiesMaterials *newMeshPropertiesMaterial = new MeshPropertiesMaterials();
  MeshPropertiesOptions *newMeshProperties = new MeshPropertiesOptions();

  makeMeshConnections(newMeshProperties,newMeshPropertiesMaterial);

  QFrame *leftFrame = genLeftFrame(newMeshPropertiesMaterial);
  QFrame *rightFrame = genRightFrame(newMeshProperties);

  QWidget *mainWidget = genMainWidget(leftFrame,rightFrame);
  tabWidget->addTab(mainWidget,"Renderview: " + QString::number(renderviewID));
  m_agent = fluo::Global::instance().getAgentFactory().getOrAddVolumePropAgent("VolumePropPanel",*this);
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

MeshPropertiesOptions* PropertiesPanel::getMeshPropertiesOptions()
{ 
  const int CURRTAB = tabWidget->currentIndex();
  QWidget* tempMain = tabWidget->widget(CURRTAB);

  MeshPropertiesOptions* temp = tempMain->findChild<MeshPropertiesOptions*>();

  return temp;
}

MeshPropertiesMaterials* PropertiesPanel::getMeshPropertiesMaterials()
{
  const int CURRTAB = tabWidget->currentIndex();
  QWidget* tempMain = tabWidget->widget(CURRTAB);

  MeshPropertiesMaterials* temp = tempMain->findChild<MeshPropertiesMaterials*>();

  return temp;
}

void PropertiesPanel::makeVolumeConnections(VolumePropertiesOptions* layout)
{
  createVolumeIntConnections(layout);
  createVolumeDblConnections(layout);
}

void PropertiesPanel::createVolumeIntConnections(VolumePropertiesOptions* layout)
{
  for(auto && tup : volumeIntConnections)
    connect(layout,std::get<0>(tup),this,std::get<1>(tup));
}

void PropertiesPanel::createVolumeDblConnections(VolumePropertiesOptions* layout)
{
  for(auto && tup : volumeDblConnections)
    connect(layout,std::get<0>(tup),this,std::get<1>(tup));
}

void PropertiesPanel::makeMeshConnections(MeshPropertiesOptions* opLayout, MeshPropertiesMaterials* maLayout)
{
  createMeshIntConnections(opLayout,maLayout);
  createMeshDblConnections(opLayout);
}

void PropertiesPanel::createMeshIntConnections(MeshPropertiesOptions* opLayout, MeshPropertiesMaterials* maLayout)
{
  connect(opLayout,&MeshPropertiesOptions::sendSizeLimitValue,this,&PropertiesPanel::onSizeLimitReceived);
  connect(maLayout,&MeshPropertiesMaterials::sendShininessValue,this,&PropertiesPanel::onShininessReceived);
  
  for(auto && tup : meshIntConnections)
    connect(opLayout,std::get<0>(tup),this,std::get<1>(tup));
}

void PropertiesPanel::createMeshDblConnections(MeshPropertiesOptions* opLayout)
{
  for(auto && tup : meshDblConnections)
    connect(opLayout,std::get<0>(tup),this,std::get<1>(tup));
}
