#include "propertiesPanel.hpp"
#include <iostream>
PropertiesPanel::PropertiesPanel()
{
  myLayout->addWidget(tabWidget);
  this->setLayout(myLayout);
}

void PropertiesPanel::onVolumeLoaded(int renderViewID)
{
  QFrame* newFrame = new QFrame();
  VolumePropertiesOptions* newVolumePropOpt = new VolumePropertiesOptions();

  newFrame->setLayout(newVolumePropOpt);
  tabWidget->addTab(newFrame,"Renderview: " + QString::number(renderViewID));
}

void PropertiesPanel::onMeshLoaded(int renderviewID)
{
    QFrame* newFrame = new QFrame();
    MeshPropertiesOptions* newVolumePropOpt = new MeshPropertiesOptions();

    newFrame->setLayout(newVolumePropOpt);
    tabWidget->addTab(newFrame,"Renderview: " + QString::number(renderviewID));
}
