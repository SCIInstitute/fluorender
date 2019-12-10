#include "meshPropertiesOptions.hpp"

MeshPropertiesOptions::MeshPropertiesOptions()
{
  addRow0();
  addRow1();
  addRow2();
  addRow3();
}

void MeshPropertiesOptions::addRow0()
{
    this->addWidget(transparencyLabel,0,1);
    this->addWidget(transparencySlider,0,2);
    this->addWidget(transparencySpinbox,0,3);
}

void MeshPropertiesOptions::addRow1()
{
    this->addWidget(shadowCheckbox,1,0);
    this->addWidget(shadowLabel,1,1);
    this->addWidget(shadowSlider,1,2);
    this->addWidget(shadowSpinbox,1,3);
}

void MeshPropertiesOptions::addRow2()
{
    this->addWidget(lightingCheckbox,2,0);
    this->addWidget(lightingLabel,2,1);
    this->addWidget(lightingSlider,2,2);
    this->addWidget(lightingSpinbox,2,3);
}

void MeshPropertiesOptions::addRow3()
{
    this->addWidget(sizeLimitCheckbox,3,0);
    this->addWidget(sizeLimitLabel,3,1);
    this->addWidget(sizeLimitSlider,3,2);
    this->addWidget(sizeLimitSpinbox,3,3);
}
