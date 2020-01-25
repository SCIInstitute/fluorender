#include "volumePropertiesOptions.hpp"

VolumePropertiesOptions::VolumePropertiesOptions()
{
  constructLayout();
  //connect(object,SIGNAL,this,SLOT(function));

  //connect(gammaSlider,SIGNAL(sliderReleased()),this,SLOT(onGammaSliderChanged()));
  connect(gammaSpinbox,SIGNAL(editingFinished()),this,SLOT(onGammaSpinboxChanged()));
  connect(extractBSlider,SIGNAL(sliderReleased()),this,SLOT(onExtBoundSliderChanged()));
  connect(extractBSpinbox,SIGNAL(editingFinished()),this,SLOT(onExtBoundSpinboxChanged()));
  connect(saturationSlider,SIGNAL(sliderReleased()),this,SLOT(onSatSliderChanged()));
  connect(saturationSpinbox,SIGNAL(editingFinished()),this,SLOT(onSatSpinboxChanged()));
  connect(threshold1Slider,SIGNAL(sliderReleased()),this,SLOT(onLowThreshSliderChanged()));
  connect(threshold1Spinbox,SIGNAL(editingFinished()),this,SLOT(onLowThreshSpinChanged()));
  connect(threshold2Slider,SIGNAL(sliderReleased()),this,SLOT(onHighThreshSliderChanged()));
  connect(threshold2Spinbox,SIGNAL(editingFinished()),this,SLOT(onHighThreshSpinChanged()));
  connect(luminanceSlider,SIGNAL(sliderReleased()),this,SLOT(onLuminanceSliderChanged()));
  connect(luminanceSpinbox,SIGNAL(editingFinished()),this,SLOT(onLuminanceSpinChanged()));
  connect(shadowSlider,SIGNAL(sliderReleased()),this,SLOT(onShadowSliderChanged()));
  connect(shadowSpinbox,SIGNAL(editingFinished()),this,SLOT(onShadowSpinChanged()));
  connect(alphaSlider,SIGNAL(sliderReleased()),this,SLOT(onAlphaSliderChanged()));
  connect(alphaSpinbox,SIGNAL(editingFinished()),this,SLOT(onAlphaSpinChanged()));
  connect(sampleRateSlider,SIGNAL(sliderReleased()),this,SLOT(onSampleSliderChanged()));
  connect(sampleRateSpinbox,SIGNAL(editingFinished()),this,SLOT(onSampleSpinChanged()));
  connect(shading1Slider,SIGNAL(sliderReleased()),this,SLOT(onLShaderSliderChanged()));
  connect(shading1Spinbox,SIGNAL(editingFinished()),this,SLOT(onLShaderSpinChanged()));
  connect(shading2Slider,SIGNAL(sliderReleased()),this,SLOT(onHShaderSliderChanged()));
  connect(shading2Spinbox,SIGNAL(editingFinished()),this,SLOT(onHShaderSpinChanged()));
  connect(colorMap1Slider,SIGNAL(sliderReleased()),this,SLOT(onLCMSliderChanged()));
  connect(colorMap1Spinbox,SIGNAL(editingFinished()),this,SLOT(onLCMSpinChanged()));
  connect(colorMap2Slider,SIGNAL(sliderReleased()),this,SLOT(onHCMSliderChanged()));
  connect(colorMap2Spinbox,SIGNAL(editingFinished()),this,SLOT(onHCMSpinChanged()));
}

void VolumePropertiesOptions::buildSliderConnections()
{

}

void VolumePropertiesOptions::buildSpinboxConnections()
{

}

void VolumePropertiesOptions::buildSpinboxDConnections()
{

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
