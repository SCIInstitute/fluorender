#include "propertiesPanel.hpp"

PropertiesPanel::PropertiesPanel()
{
  myLayout->addWidget(tabWidget);
  this->setLayout(myLayout);
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
