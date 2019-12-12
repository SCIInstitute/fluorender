#include "volumePropertiesMisc.hpp"

VolumePropertiesMisc::VolumePropertiesMisc()
{
  addRow0();
  addRow1();
  addRow2();
  addRow3();
}

void VolumePropertiesMisc::addRow0()
{
  this->addWidget(voxelSizeLabel,0,0);
  this->addWidget(xLabel,0,1);
  this->addWidget(xSpinbox,0,2);
  this->addWidget(yLabel,0,3);
  this->addWidget(ySpinbox,0,4);
  this->addWidget(zLabel,0,5);
  this->addWidget(zSpinbox,0,6);
}

void VolumePropertiesMisc::addRow1()
{
  this->addWidget(primeColorLabel,1,0);
  this->addWidget(primeRLabel,1,1);
  this->addWidget(primeRSpinbox,1,2);
  this->addWidget(primeGLabel,1,3);
  this->addWidget(primeGSpinbox,1,4);
  this->addWidget(primeBLabel,1,5);
  this->addWidget(primeBSpinbox,1,6);
  this->addWidget(primeColorButton,1,7);
}

void VolumePropertiesMisc::addRow2()
{
  this->addWidget(secondColorLabel,2,0);
  this->addWidget(secondRLabel,2,1);
  this->addWidget(secondRSpinbox,2,2);
  this->addWidget(secondGLabel,2,3);
  this->addWidget(secondGSpinbox,2,4);
  this->addWidget(secondBLabel,2,5);
  this->addWidget(secondBSpinbox,2,6);
  this->addWidget(secondColorButton,2,7);
}

void VolumePropertiesMisc::addRow3()
{
  this->addWidget(effectsLabel,3,0);
  this->addWidget(effectsToolButton,3,1);
  this->addWidget(effectsList1,3,2,1,3);
  this->addWidget(effectsList2,3,5,1,3);
}

QComboBox* VolumePropertiesMisc::genComboBox(const QStringList &list)
{
  QComboBox *newCombobox = new QComboBox();
  newCombobox->addItems(list);

  return newCombobox;
}
