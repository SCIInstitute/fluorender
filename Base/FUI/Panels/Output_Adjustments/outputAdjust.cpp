#include "outputAdjust.hpp"
#include <Global/Global.hpp>
#include <Base_Agent/AgentFactory.hpp>
#include <Global/Names.hpp>

OutputAdjustments::OutputAdjustments()
{
  outputFrame->setLayout(outputLayout);
  frameLayout->addWidget(outputFrame);
  this->setLayout(frameLayout);

  makeIntConnections();
  makeDblConnections();
  this->outputLayout->disableLayout();
}

void OutputAdjustments::setOutRedLuminValue(double newVal)
{
  this->outputLayout->setRedLuminValue(newVal);
}

void OutputAdjustments::setOutGreenLuminValue(double newVal)
{
  this->outputLayout->setGreenLuminValue(newVal);
}

void OutputAdjustments::setOutBlueLuminValue(double newVal)
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
	fluo::Object* obj = fluo::Global::instance().get(flstrAgentFactory);
	if (!obj)
		return;
	m_agent = dynamic_cast<AgentFactory*>(obj)->getOrAddOutAdjustAgent("OutAdjustPanel", this);
	//m_agent = fluo::Global::instance().getAgentFactory().getOrAddOutAdjustAgent("OutAdjustPanel",this);
  this->outputLayout->setAgent(m_agent,vd);
  this->outputLayout->buildWrappers();
  this->outputLayout->buildControllers();
  this->outputLayout->enableLayout();
  m_agent->UpdateAllSettings();
}
