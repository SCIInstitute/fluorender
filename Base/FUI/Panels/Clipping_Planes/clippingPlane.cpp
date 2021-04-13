#include "clippingPlane.hpp"
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <Names.hpp>

ClippingPlane::ClippingPlane()
{
  createLayout();
  makeIntConnections();
  //makeDblConnections();
  makeBoolConnections();
  clippingLayout->disableLayout();
}

void ClippingPlane::createLayout()
{
  outputFrame->setLayout(clippingLayout);
  frameLayout->addWidget(outputFrame);
  this->setLayout(frameLayout);
}

void ClippingPlane::setClipSalmonValue(int newVal)
{
  clippingLayout->setSalmonValue(newVal);
}

void ClippingPlane::setClipMagentaValue(int newVal)
{
  clippingLayout->setMagentaValue(newVal);
}

void ClippingPlane::setClipGreenValue(int newVal)
{
  clippingLayout->setGreenValue(newVal);
}

void ClippingPlane::setClipYellowValue(int newVal)
{
  clippingLayout->setYellowValue(newVal);
}

void ClippingPlane::setClipPurpleValue(int newVal)
{
  clippingLayout->setPurpleValue(newVal);
}

void ClippingPlane::setClipTealValue(int newVal)
{
  clippingLayout->setTealValue(newVal);
}

void ClippingPlane::setClipClipValue(int newVal)
{
  clippingLayout->setClipValue(newVal);
}

void ClippingPlane::setClipSlabValue(int newVal)
{
  clippingLayout->setSlabValue(newVal);
}

void ClippingPlane::setClipWidthValue(int newVal)
{
  clippingLayout->setWidthValue(newVal);
}

void ClippingPlane::setXPlaneLockStatus(bool status)
{
  clippingLayout->setXPlaneBool(status);
}

void ClippingPlane::setYPlaneLockStatus(bool status)
{
  clippingLayout->setYPlaneBool(status);
}

void ClippingPlane::setZPlaneLockStatus(bool status)
{
  clippingLayout->setZPlaneBool(status);
}

const int ClippingPlane::getClipSalmonMaxVal()
{
  return clippingLayout->getSalmonSliderMax();
}

const int ClippingPlane::getClipGreenMaxVal()
{
  return clippingLayout->getGreenSliderMax();
}

const int ClippingPlane::getClipPurpleMaxVal()
{
  return clippingLayout->getPurpleSliderMax();
}

/*
void ClippingPlane::makeDblConnections()
{
  for(auto &&tup : dblConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}
*/
void ClippingPlane::makeBoolConnections()
{
  for(auto &&tup : boolConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void ClippingPlane::makeIntConnections()
{
  for(auto &&tup : intConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}

void ClippingPlane::setVolumeData(fluo::VolumeData* vd)
{
	AgentFactory* factory = fluo::Global::instance().getAgentFactory();
	if (!factory)
		return;
  m_agent = factory->getOrAddClipPlaneAgent("ClipPlanePanel",*this);
  clippingLayout->setAgent(m_agent,vd);
  clippingLayout->build();
  clippingLayout->enableLayout();
  m_agent->UpdateAllSettings();
}
