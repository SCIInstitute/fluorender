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
  x1MSlider->setInvertedAppearance(true);
  y1GSlider->setInvertedAppearance(true);
  y1YSlider->setInvertedAppearance(true);
  z1PSlider->setInvertedAppearance(true);
  z1TSlider->setInvertedAppearance(true);
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

void ClippingLayout::setAgent(ClipPlaneAgent* agent, fluo::VolumeData* vd)
{
  m_agent = agent;
  m_agent->setObject(vd);
  buildWrappers();
  buildControllers();
}

void ClippingLayout::disableLayout()
{
  for(auto && tup : sliderConnections)
    std::get<0>(tup)->setEnabled(false);
  
  for(auto && tup : spinConnections)
    std::get<0>(tup)->setEnabled(false);
  
  for(auto && tup : dSpinConnections)
    std::get<0>(tup)->setEnabled(false);

  disableButtons();
}

void ClippingLayout::enableLayout()
{
  for(auto && tup : sliderConnections)
    std::get<0>(tup)->setEnabled(true);
  
  for(auto && tup : spinConnections)
    std::get<0>(tup)->setEnabled(true);
  
  for(auto && tup : dSpinConnections)
    std::get<0>(tup)->setEnabled(true);

  enableButtons();
}

void ClippingLayout::disableButtons()
{
  xChainToolbutton->setEnabled(false);
  yChainToolbutton->setEnabled(false);
  zChainToolbutton->setEnabled(false);
  yzButton->setEnabled(false);
  xzButton->setEnabled(false);
  xyButton->setEnabled(false);
  resetClipsButton->setEnabled(false);
  alignToViewButton->setEnabled(false);
  resetTo0Button->setEnabled(false);
}

void ClippingLayout::enableButtons()
{
  xChainToolbutton->setEnabled(true);
  yChainToolbutton->setEnabled(true);
  zChainToolbutton->setEnabled(true);
  yzButton->setEnabled(true);
  xzButton->setEnabled(true);
  xyButton->setEnabled(true);
  resetClipsButton->setEnabled(true);
  alignToViewButton->setEnabled(true);
  resetTo0Button->setEnabled(true);
}

void ClippingLayout::build()
{
  buildWrappers();
  buildControllers();
}

void ClippingLayout::buildWrappers()
{
  salmonWrapper = new AgentWrapper<ClipPlaneAgent>("clip x1", m_agent);
  magentaWrapper = new AgentWrapper<ClipPlaneAgent>("clip x2", m_agent);
  greenWrapper = new AgentWrapper<ClipPlaneAgent>("clip y1", m_agent);
  yellowWrapper = new AgentWrapper<ClipPlaneAgent>("clip y2", m_agent);
  purpleWrapper = new AgentWrapper<ClipPlaneAgent>("clip z1", m_agent);
  tealWrapper = new AgentWrapper<ClipPlaneAgent>("clip z2", m_agent);
  // need to create agents for the distance. However I don't think I have the proper
  // logic set up for the XY, YZ, XZ spin box.
  clipLinkXWrapper = new AgentWrapper<ClipPlaneAgent>("clip dist x", m_agent);
  clipLinkYWrapper = new AgentWrapper<ClipPlaneAgent>("clip dist y", m_agent);
  clipLinkZWrapper = new AgentWrapper<ClipPlaneAgent>("clip dist z", m_agent);
  clipRotXWrapper = new AgentWrapper<ClipPlaneAgent>("clip rot x", m_agent);
  clipRotYWrapper = new AgentWrapper<ClipPlaneAgent>("clip rot y", m_agent);
  clipRotZWrapper = new AgentWrapper<ClipPlaneAgent>("clip rot z", m_agent);
}

void ClippingLayout::buildControllers()
{
  x1SalmonController = new Controller<FluoSpinbox, FluoSlider, AgentWrapper<ClipPlaneAgent>>(*x1SSpinbox,*x1SSlider,*salmonWrapper);
  x1MagentaController = new Controller<FluoSpinbox, FluoSlider, AgentWrapper<ClipPlaneAgent>>(*x1MSpinbox,*x1MSlider,*magentaWrapper);
  y1GreenController = new Controller<FluoSpinbox, FluoSlider, AgentWrapper<ClipPlaneAgent>>(*y1GSpinbox,*y1GSlider,*greenWrapper);
  y1YellowController = new Controller<FluoSpinbox, FluoSlider, AgentWrapper<ClipPlaneAgent>>(*y1YSpinbox,*y1YSlider,*yellowWrapper);
  z1PurpleController = new Controller<FluoSpinbox, FluoSlider, AgentWrapper<ClipPlaneAgent>>(*z1PSpinbox,*z1PSlider,*purpleWrapper);
  z1TealController = new Controller<FluoSpinbox, FluoSlider, AgentWrapper<ClipPlaneAgent>>(*z1TSpinbox,*z1TSlider,*tealWrapper);
  x2RotationController = new Controller<FluoSpinboxDouble, FluoSlider, AgentWrapper<ClipPlaneAgent>>(*x2Spinbox,*x2Slider,*clipRotXWrapper);
}

std::tuple<int,int> ClippingLayout::sliderGap(FluoSlider* s1, FluoSlider *s2, int size)
{
  const int gapValue = setClipSpinbox->value();

  return std::make_tuple(HALFXY - size/2, HALFXY + size/2);
}

void ClippingLayout::setSalmonValue(int newVal)
{
  if (xChainToolbutton->isChecked())
  {
    x1SalmonController->setValues(newVal);
    x1MagentaController->setValues(x1MSlider->value()+newVal);
  }
  else
    if(x1SSlider->value() <= x1MSlider->value())
      x1SalmonController->setValues(newVal);
}

void ClippingLayout::setMagentaValue(int newVal)
{
  if (xChainToolbutton->isChecked())
  {
    x1MagentaController->setValues(newVal);
    x1SalmonController->setValues(newVal - x1SSlider->value());
  }
  else
    if(x1MSlider->value() >= x1SSlider->value())
      x1MagentaController->setValues(newVal);

}

void ClippingLayout::setGreenValue(int newVal)
{
  if (yChainToolbutton->isChecked())
  {
    y1GreenController->setValues(newVal);
    y1YellowController->setValues(y1YSlider->value() + newVal);
  }
  else
  {
    if(y1GSlider->value() <= y1YSlider->value())
      y1GreenController->setValues(newVal);
  }
}

void ClippingLayout::setYellowValue(int newVal)
{
  if (yChainToolbutton->isChecked())
  {
    y1YellowController->setValues(newVal);
    y1GreenController->setValues(newVal - y1GSlider->value());
  }
  else
  {
    if(y1YSlider->value() >= y1GSlider->value())
      y1YellowController->setValues(newVal);
  }

}

void ClippingLayout::setPurpleValue(int newVal)
{
  if (zChainToolbutton->isChecked())
  {
    z1PurpleController->setValues(newVal);
    z1TealController->setValues(z1TSlider->value() + newVal);
  }
  else
  {
    if(z1PSlider->value() <= z1TSlider->value())
      z1PurpleController->setValues(newVal);
  }
}

void ClippingLayout::setTealValue(int newVal)
{
  if (zChainToolbutton->isChecked())
  {
    z1TealController->setValues(newVal);
    z1PurpleController->setValues(newVal - z1PSlider->value());
  }
  else
  {
    if(z1TSlider->value() >= z1PSlider->value())
      z1TealController->setValues(newVal);
  }
}
