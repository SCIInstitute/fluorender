#include "meshPropertiesMaterials.hpp"

MeshPropertiesMaterials::MeshPropertiesMaterials()
{
  addRow0();
  addRow1();
  addRow2();

  connect(shininessSlider,&FluoSlider::sliderReleased,this,&MeshPropertiesMaterials::onShininessSliderChanged);
  connect(shininessSpinBox,&FluoSpinbox::editingFinished,this,&MeshPropertiesMaterials::onShininessSpinboxChanged);
}

void MeshPropertiesMaterials::addRow0()
{
  this->addWidget(diffuseColorLabel,0,0);
  this->addWidget(diffuseColorButton,0,1);
  //this->addWidget(diffuseColorLabel,0,2);
}

void MeshPropertiesMaterials::addRow1()
{
  this->addWidget(specularColorLabel,1,0);
  this->addWidget(specularColorButton,1,1);
}

void MeshPropertiesMaterials::addRow2()
{
  this->addWidget(shininessLabel,2,0);
  this->addWidget(shininessSlider,2,1);
  this->addWidget(shininessSpinBox,2,2);
}
