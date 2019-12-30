#include "volumePropertiesOptions.hpp"

VolumePropertiesOptions::VolumePropertiesOptions()
{
  constructLayout();
}

void VolumePropertiesOptions::addRow0()
{
  this->addWidget(gammaSlider,0,0,1,3);
  this->addWidget(gammaSpinbox,0,3);
  this->addWidget(gammaLabel,0,4);
  this->addWidget(extractBLabel,0,5);
  this->addWidget(extractBSpinbox,0,6);
  this->addWidget(extractBSlider,0,7,1,3);
}

void VolumePropertiesOptions::addRow1()
{
  this->addWidget(saturationSlider,1,0,1,3);
  this->addWidget(saturationSpinbox,1,3);
  this->addWidget(saturationsLabel,1,4);
  this->addWidget(thresholdLabel,1,5);
  this->addWidget(threshold1Spinbox,1,6);
  this->addWidget(threshold1Slider,1,7);
  this->addWidget(threshold2Spinbox,1,8);
  this->addWidget(threshold2Slider,1,9);
}

void VolumePropertiesOptions::addRow2()
{
  this->addWidget(luminanceSlider,2,0,1,3);
  this->addWidget(luminanceSpinbox,2,3);
  this->addWidget(luminanceLabel,2,4);
  this->addWidget(shadowLabel,2,5);
  this->addWidget(shadowSpinbox,2,6);
  this->addWidget(shadowSlider,2,7,1,3);
}

void VolumePropertiesOptions::addRow3()
{
  this->addWidget(alphaSlider,3,0,1,3);
  this->addWidget(alphaSpinbox,3,3);
  this->addWidget(alphaLabel,3,4);
  this->addWidget(sampleRateLabel,3,5);
  this->addWidget(sampleRateSpinbox,3,6);
  this->addWidget(sampleRateSlider,3,7,1,3);
}

void VolumePropertiesOptions::addRow4()
{
  this->addWidget(shading1Slider,4,0);
  this->addWidget(shading1Spinbox,4,1);
  this->addWidget(shading2Slider,4,2);
  this->addWidget(shading2Spinbox,4,3);
  this->addWidget(shadingLabel,4,4);
  this->addWidget(colorMapLabel,4,5);
  this->addWidget(colorMap1Spinbox,4,6);
  this->addWidget(colorMap1Slider,4,7);
  this->addWidget(colorMap2Spinbox,4,8);
  this->addWidget(colorMap2Slider,4,9);
}

void VolumePropertiesOptions::constructLayout()
{
  for (const auto &fn : rowFuncs)
    fn();
}
