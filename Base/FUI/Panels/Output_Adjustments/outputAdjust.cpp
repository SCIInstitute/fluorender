#include "outputAdjust.hpp"
#include <Global/Global.hpp>

OutputAdjustments::OutputAdjustments()
{
  outputFrame->setLayout(outputLayout);
  frameLayout->addWidget(outputFrame);
  this->setLayout(frameLayout);

  makeStdAnyConnections();
  makeIntConnections();

  m_agent = fluo::Global::instance().getAgentFactory().getOrAddOutAdjustAgent("OutAdjustPanel",*this);
}

void OutputAdjustments::setOutRedLuminValue(int newVal)
{
  OutputLayout* temp = getOutputLayout();
  temp->setRedLuminValue(newVal);
}

void OutputAdjustments::setOutGreenLuminValue(int newVal)
{
  OutputLayout* temp = getOutputLayout();
  temp->setGreenLuminValue(newVal);
}

void OutputAdjustments::setOutBlueLuminValue(int newVal)
{
  OutputLayout* temp = getOutputLayout();
  temp->setBlueLuminValue(newVal);
}

OutputLayout* OutputAdjustments::getOutputLayout()
{
  OutputLayout* temp = this->findChild<OutputLayout*>();
  return temp;
}

void OutputAdjustments::makeStdAnyConnections()
{
  for(auto && tup : stdAnyConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void OutputAdjustments::makeIntConnections()
{
  for(auto && tup : intConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}
