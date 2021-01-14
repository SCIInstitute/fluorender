#include "outputLayout.hpp"

OutputLayout::OutputLayout()
{
  constructLayout();
  buildSliderConnections();
  buildSpinboxConnections();
  buildSpinboxDConnections();
}

void OutputLayout::constructLayout()
{
  for(const auto &fn : rowFuncs)
    fn();
}

void OutputLayout::buildSliderConnections()
{
  for(auto && tup: sliderConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));

  for(auto && tup: luminSliderConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputLayout::buildSpinboxConnections()
{
  for(auto && tup: luminSpinConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputLayout::buildSpinboxDConnections()
{
  for(auto && tup: dSpinConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputLayout::enableLayout()
{
  for(auto && tup : sliderConnections)
    std::get<0>(tup)->setEnabled(true);

  for(auto && tup : luminSliderConnections)
    std::get<0>(tup)->setEnabled(true);
  
  for(auto && tup : luminSpinConnections)
    std::get<0>(tup)->setEnabled(true);
  
  for(auto && tup : dSpinConnections)
    std::get<0>(tup)->setEnabled(true);
}

void OutputLayout::disableLayout()
{
  for(auto && tup : sliderConnections)
    std::get<0>(tup)->setEnabled(false);
  
  for(auto && tup : luminSliderConnections)
    std::get<0>(tup)->setEnabled(false);
  
  for(auto && tup : luminSpinConnections)
    std::get<0>(tup)->setEnabled(false);
  
  for(auto && tup : dSpinConnections)
    std::get<0>(tup)->setEnabled(false);
}

void OutputLayout::buildWrappers()
{
  redGammaWrapper = new AgentWrapper<OutAdjustAgent>("gamma r", m_agent);
  greenGammaWrapper = new AgentWrapper<OutAdjustAgent>("gamma g", m_agent);
  blueGammaWrapper = new AgentWrapper<OutAdjustAgent>("gamma b", m_agent);
  redLuminWrapper = new AgentWrapper<OutAdjustAgent>("brightness r", m_agent);
  greenLuminWrapper = new AgentWrapper<OutAdjustAgent>("brightness g", m_agent);
  blueLuminWrapper = new AgentWrapper<OutAdjustAgent>("brightness b", m_agent);
  redEqlWrapper = new AgentWrapper<OutAdjustAgent>("equalize r", m_agent);
  greenEqlWrapper = new AgentWrapper<OutAdjustAgent>("equalize g", m_agent);
  blueEqlWrapper = new AgentWrapper<OutAdjustAgent>("equalize b", m_agent);
}

void OutputLayout::buildControllers()
{
    redGammaController = new Controller<FluoSlider, FluoSpinboxDouble, AgentWrapper<OutAdjustAgent>>(*redGammaSlider, *redGammaSpinbox, *redGammaWrapper);
    greenGammaController = new Controller<FluoSlider, FluoSpinboxDouble, AgentWrapper<OutAdjustAgent>>(*greenGammaSlider, *greenGammaSpinbox, *greenGammaWrapper);
    blueGammaController =  new Controller<FluoSlider, FluoSpinboxDouble, AgentWrapper<OutAdjustAgent>>(*blueGammaSlider, *blueGammaSpinbox, *blueGammaWrapper);
    redLuminController = new Controller<FluoLuminSlider, FluoLuminSpinbox, AgentWrapper<OutAdjustAgent>>(*redLuminSlider, *redLuminSpinbox, *redLuminWrapper);
    greenLuminController = new Controller<FluoLuminSlider,FluoLuminSpinbox, AgentWrapper<OutAdjustAgent>>(*greenLuminSlider, *greenLuminSpinbox, *greenLuminWrapper);
    blueLuminController = new Controller<FluoLuminSlider,FluoLuminSpinbox, AgentWrapper<OutAdjustAgent>>(*blueLuminSlider,*blueLuminSpinbox, *blueLuminWrapper);
    redEqlController = new Controller<FluoSlider,FluoSpinboxDouble, AgentWrapper<OutAdjustAgent>>(*redEqlSlider,*redEqlSpinbox,*redEqlWrapper);
    greenEqlController = new Controller<FluoSlider,FluoSpinboxDouble, AgentWrapper<OutAdjustAgent>>(*greenEqlSlider,*greenEqlSpinbox,*greenEqlWrapper);
    blueEqlController = new Controller<FluoSlider,FluoSpinboxDouble, AgentWrapper<OutAdjustAgent>>(*blueEqlSlider,*blueEqlSpinbox,*blueEqlWrapper);

}
