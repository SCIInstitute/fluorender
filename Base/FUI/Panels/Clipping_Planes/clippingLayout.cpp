#include "clippingLayout.hpp"

ClippingLayout::ClippingLayout()
{
  flipSliders();
  constructLayout();
  buildSliderConnections();
  buildSpinboxConnections();
  buildSpinboxDConnections();
}

void ClippingLayout::flipSliders()
{
  x1SSlider->setInvertedAppearance(true);
  y1GSlider->setInvertedAppearance(true);
  z1PSlider->setInvertedAppearance(true);
}

void ClippingLayout::row0()
{
  this->addWidget(linkToolButton,0,3);
  this->addWidget(alwaysToolButton,0,4);
  this->addWidget(toggleToolButton,0,5);
}

void ClippingLayout::constructLayout()
{
  for(const auto &fn : rowFuncs)
    fn();
}

void ClippingLayout::buildSliderConnections()
{
  for(auto && tup : sliderConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void ClippingLayout::buildSpinboxConnections()
{
  for(auto && tup : spinConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void ClippingLayout::buildSpinboxDConnections()
{
  for(auto && tup : dSpinConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}
