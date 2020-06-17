#include "outputAdjust.hpp"
#include <Global/Global.hpp>

OutputAdjustments::OutputAdjustments()
{
  outputFrame->setLayout(outputLayout);
  frameLayout->addWidget(outputFrame);
  this->setLayout(frameLayout);

  makeIntConnections();
  makeDblConnections();

  m_agent = fluo::Global::instance().getAgentFactory().getOrAddOutAdjustAgent("OutAdjustPanel",*this);
}

void OutputAdjustments::setOutRedLuminValue(int newVal)
{
  outputLayout->setRedLuminValue(newVal);
}

void OutputAdjustments::setOutGreenLuminValue(int newVal)
{
  outputLayout->setGreenLuminValue(newVal);
}

void OutputAdjustments::setOutBlueLuminValue(int newVal)
{
  outputLayout->setBlueLuminValue(newVal);
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
