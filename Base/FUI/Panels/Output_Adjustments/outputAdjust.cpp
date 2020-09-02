#include "outputAdjust.hpp"
#include <Global/Global.hpp>

OutputAdjustments::OutputAdjustments()
{
  outputFrame->setLayout(outputLayout);
  frameLayout->addWidget(outputFrame);
  this->setLayout(frameLayout);

  makeIntConnections();
  makeDblConnections();
  this->outputLayout->disableLayout();
}

void OutputAdjustments::setOutRedLuminValue(int newVal)
{
  this->outputLayout->setRedLuminValue(newVal);
}

void OutputAdjustments::setOutGreenLuminValue(int newVal)
{
  this->outputLayout->setGreenLuminValue(newVal);
}

void OutputAdjustments::setOutBlueLuminValue(int newVal)
{
  this->outputLayout->setBlueLuminValue(newVal);
}

void OutputAdjustments::makeIntConnections()
{
  for(auto && tup : intConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputAdjustments::makeDblConnections()
{
  for(auto && tup : dblConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputAdjustments::setVolumeData(fluo::VolumeData* vd)
{
  m_agent = fluo::Global::instance().getAgentFactory().getOrAddOutAdjustAgent("OutAdjustPanel",this);
  this->outputLayout->setAgent(m_agent,vd);
  this->outputLayout->buildWrappers();
  this->outputLayout->buildControllers();
  this->outputLayout->enableLayout();
  m_agent->UpdateAllSettings();
}
