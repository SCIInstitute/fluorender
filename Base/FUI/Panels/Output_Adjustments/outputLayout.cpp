#include "outputLayout.hpp"

OutputLayout::OutputLayout()
{
  addRow(0,gammaLabel,luminanceLabel,equalLabel);
  addRow(1,redLabel,redSepLine,redChainButton);
  addSingle(2,redLine);
  addRow(3,redGammaSlider,redLuminSlider,redEqlSlider);
  addSingle(4,redResetButton);
  addRow(5,redGammaSpinbox,redLuminSpinbox,redEqlSpinbox);
  addRow(6,greenLabel,greenSepLine,greenChainButton);
  addSingle(7,greenLine);
  addRow(8,greenGammaSlider,greenLuminSlider,greenEqlSlider);
  addSingle(9,greenResetButton);
  addRow(10,greenGammaSpinbox,greenLuminSpinbox,greenEqlSpinbox);
  addRow(11,blueLabel,blueSepLine,blueChainButton);
  addSingle(12,blueLine);
  addRow(13,blueGammaSlider,blueLuminSlider,blueEqlSlider);
  addSingle(14,blueResetButton);
  addRow(15,blueGammaSpinbox,blueLuminSpinbox,blueEqlSpinbox);
  addSingle(16,setDefaultLine);
  addSingle(17,resetAllButton);

  buildSliderConnections();
  buildSpinboxConnections();
  buildSpinboxDConnections();
}

void OutputLayout::buildSliderConnections()
{
  for(auto && tup: sliderConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputLayout::buildSpinboxConnections()
{
  for(auto && tup: spinConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputLayout::buildSpinboxDConnections()
{
  for(auto && tup: dSpinConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}
