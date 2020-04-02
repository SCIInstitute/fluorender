#include "clippingLayout.hpp"

ClippingLayout::ClippingLayout()
{
  constructLayout();
/*
  row0();
  addSingle(1,lineSep1);
  addThreeToRowShift(2,x1Label,y1Label,z1Label);
  addThreeToRow(3,x1SSpinbox,y1GSpinbox,z1PSpinbox);
  addThreeToRow(4,salmonHLine,greenHLine,purpleHLine);
  addRow(5,x1SSlider,salmonVLine,x1MSlider,y1GSlider,greenVLine,y1YSlider,z1PSlider,purpleVLine,z1TSlider);
  addThreeToRow(6,magentaHLine,yellowHLine,tealHLine);
  addThreeToRowShift(7,xChainToolbutton,yChainToolbutton,zChainToolbutton);
  addSingle(8,lineSep2);
  addThreeToRowShift(9,setClipLabel,slabLabel,widthLabel);
  addThreeToRow(10,yzButton,xzButton,xyButton);
  addThreeToRow(11,setClipSpinbox,slabSpinbox,widthSpinbox);
  addSingle(12,resetClipsButton);
  addSingle(13,lineSep3);
  addSingle(14,rotationsLabel);
  addSingle(15,alignToViewButton);
  addSingle(16,resetTo0Button);
  addThreeToRowShift(17,x2Label,y2Label,z2Label);
  addThreeToRow(18,z2Spinbox,y2Spinbox,x2Spinbox);
  addThreeToRowShift(19,z2Slider,y2Slider,x2Slider);
*/
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
