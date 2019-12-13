#include "volumePropertiesMisc.hpp"

VolumePropertiesMisc::VolumePropertiesMisc()
{
  addRow0();
  addRow1();
  addRow2();
  addRow3();
  addRow4();
  addRow5();
}

void VolumePropertiesMisc::addRow0()
{
  this->addWidget(toolButtonWidget,0,0,1,9);
}

void VolumePropertiesMisc::addRow1()
{
  this->addWidget(line,1,0,1,9);
}

void VolumePropertiesMisc::addRow2()
{
  this->addWidget(voxelSizeLabel,2,0);
  this->addWidget(xLabel,2,1);
  this->addWidget(xSpinbox,2,2);
  this->addWidget(yLabel,2,3);
  this->addWidget(ySpinbox,2,4);
  this->addWidget(zLabel,2,5);
  this->addWidget(zSpinbox,2,6);
}

void VolumePropertiesMisc::addRow3()
{
  this->addWidget(primeColorLabel,3,0);
  this->addWidget(primeRLabel,3,1);
  this->addWidget(primeRSpinbox,3,2);
  this->addWidget(primeGLabel,3,3);
  this->addWidget(primeGSpinbox,3,4);
  this->addWidget(primeBLabel,3,5);
  this->addWidget(primeBSpinbox,3,6);
  this->addWidget(primeColorButton,3,7);
}

void VolumePropertiesMisc::addRow4()
{
  this->addWidget(secondColorLabel,4,0);
  this->addWidget(secondRLabel,4,1);
  this->addWidget(secondRSpinbox,4,2);
  this->addWidget(secondGLabel,4,3);
  this->addWidget(secondGSpinbox,4,4);
  this->addWidget(secondBLabel,4,5);
  this->addWidget(secondBSpinbox,4,6);
  this->addWidget(secondColorButton,4,7);
}

void VolumePropertiesMisc::addRow5()
{
  this->addWidget(effectsLabel,5,0);
  this->addWidget(effectsToolButton,5,1);
  this->addWidget(effectsList1,5,2,1,3);
  this->addWidget(effectsList2,5,5,1,3);
}

QWidget* VolumePropertiesMisc::genToolWidget()
{
  QWidget *toolWidget = new QWidget();
  QGridLayout *toolLayout = populateToolLayout();

  toolWidget->setLayout(toolLayout);

  return toolWidget;
}

QComboBox* VolumePropertiesMisc::genComboBox(const QStringList &list)
{
  QComboBox *newCombobox = new QComboBox();
  newCombobox->addItems(list);

  return newCombobox;
}

QFrame* VolumePropertiesMisc::genLine()
{
  QFrame *newLine = new QFrame();
  newLine->setFrameShape(QFrame::HLine);
  newLine->setFrameShadow(QFrame::Sunken);

  return newLine;
}

QGridLayout* VolumePropertiesMisc::populateToolLayout()
{
  QGridLayout *toolLayout = new QGridLayout();

  toolLayout->addWidget(highTransparancyButton,0,0);
  toolLayout->addWidget(mIPButton,0,1);
  toolLayout->addWidget(invertDataButton,0,2);
  toolLayout->addWidget(spatialInterpolationButton,0,3);
  toolLayout->addWidget(renderingResultButton,0,4);
  toolLayout->addWidget(syncChannelButton,0,5);
  toolLayout->addWidget(depthModeButton,0,6);
  toolLayout->addWidget(nameLegendButton,0,7);
  toolLayout->addWidget(resetAllButton,0,8);
  toolLayout->addWidget(saveDefaultsButton,0,9);

  return toolLayout;
}

