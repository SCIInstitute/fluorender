#include "outputAdjust.hpp"
#include <Global/Global.hpp>

OutputAdjustments::OutputAdjustments()
{
  outputFrame->setLayout(outputLayout);
  frameLayout->addWidget(outputFrame);
  this->setLayout(frameLayout);

  m_agent = fluo::Global::instance().getAgentFactory().getOrAddOutAdjustAgent("OutAdjustPanel",*this);
}

void OutputAdjustments::setRedLuminValue(int newVal)
{
  OutputLayout* temp = getOutputLayout();
  temp->setRedLuminValue(newVal);
}

void OutputAdjustments::setGreenLuminValue(int newVal)
{
  OutputLayout* temp = getOutputLayout();
  temp->setGreenLuminValue(newVal);
}

void OutputAdjustments::setBlueLuminValue(int newVal)
{
  OutputLayout* temp = getOutputLayout();
  temp->setBlueLuminValue(newVal);
}

OutputLayout* OutputAdjustments::getOutputLayout()
{
  OutputLayout* temp = this->findChild<OutputLayout*>();
  return temp;
}
