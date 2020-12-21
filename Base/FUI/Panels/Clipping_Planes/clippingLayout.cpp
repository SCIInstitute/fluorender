#include "clippingLayout.hpp"

ClippingLayout::ClippingLayout()
{
  flipSliders();
  constructLayout();
  buildSliderConnections();
  buildSpinboxConnections();
  //buildSpinboxDConnections();
  buildButtonConnections();
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

/*
void ClippingLayout::buildSpinboxDConnections()
{
  for(auto && tup : dSpinConnections)
    connect(std::get<0>(tup),std::get<1>(tup),this,std::get<2>(tup));
}
*/

void ClippingLayout::buildButtonConnections()
{
  connect(yzButton,&QPushButton::released,this,&ClippingLayout::onYZButtonPushed);
  connect(xzButton,&QPushButton::released,this,&ClippingLayout::onXZButtonPushed);
  connect(xyButton,&QPushButton::released,this,&ClippingLayout::onXYButtonPushed);
}

void ClippingLayout::setAgent(ClipPlaneAgent* agent, fluo::VolumeData* vd)
{
  m_agent = agent;
  m_agent->setObject(vd);
}

void ClippingLayout::disableLayout()
{
  for(auto && tup : sliderConnections)
    std::get<0>(tup)->setEnabled(false);
  
  for(auto && tup : spinConnections)
    std::get<0>(tup)->setEnabled(false);
 /*
  for(auto && tup : dSpinConnections)
    std::get<0>(tup)->setEnabled(false);
*/
  disableButtons();
}

void ClippingLayout::enableLayout()
{
  for(auto && tup : sliderConnections)
    std::get<0>(tup)->setEnabled(true);
  
  for(auto && tup : spinConnections)
    std::get<0>(tup)->setEnabled(true);
 /*
  for(auto && tup : dSpinConnections)
    std::get<0>(tup)->setEnabled(true);
*/
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
  buildTuples();
}

void ClippingLayout::buildWrappers()
{
  salmonWrapper = new AgentWrapper<ClipPlaneAgent>("clip x1", m_agent);
  magentaWrapper = new AgentWrapper<ClipPlaneAgent>("clip x2", m_agent);
  greenWrapper = new AgentWrapper<ClipPlaneAgent>("clip y1", m_agent);
  yellowWrapper = new AgentWrapper<ClipPlaneAgent>("clip y2", m_agent);
  purpleWrapper = new AgentWrapper<ClipPlaneAgent>("clip z1", m_agent);
  tealWrapper = new AgentWrapper<ClipPlaneAgent>("clip z2", m_agent);
  clipLinkXWrapper = new AgentWrapper<ClipPlaneAgent>("clip dist x", m_agent);
  clipLinkYWrapper = new AgentWrapper<ClipPlaneAgent>("clip dist y", m_agent);
  clipLinkZWrapper = new AgentWrapper<ClipPlaneAgent>("clip dist z", m_agent);
  clipRotXWrapper = new AgentWrapper<ClipPlaneAgent>("clip rot x", m_agent);
  clipRotYWrapper = new AgentWrapper<ClipPlaneAgent>("clip rot y", m_agent);
  clipRotZWrapper = new AgentWrapper<ClipPlaneAgent>("clip rot z", m_agent);
}

void ClippingLayout::buildControllers()
{
  x1SalmonController = new ControllerInt(*x1SSpinbox,*x1SSlider,*salmonWrapper);
  x1MagentaController = new ControllerInt(*x1MSpinbox,*x1MSlider,*magentaWrapper);
  y1GreenController = new ControllerInt(*y1GSpinbox,*y1GSlider,*greenWrapper);
  y1YellowController = new ControllerInt(*y1YSpinbox,*y1YSlider,*yellowWrapper);
  z1PurpleController = new ControllerInt(*z1PSpinbox,*z1PSlider,*purpleWrapper);
  z1TealController = new ControllerInt(*z1TSpinbox,*z1TSlider,*tealWrapper);
  //x2RotationController = new ControllerDbl(*x2Spinbox,*x2Slider,*clipRotXWrapper);
  //y2RotationController = new ControllerDbl(*y2Spinbox,*y2Slider,*clipRotYWrapper);
  //z2RotationController = new ControllerDbl(*z2Spinbox,*z2Slider,*clipRotZWrapper);
  x2RotationController = new ControllerInt(*x2Spinbox,*x2Slider,*clipRotXWrapper);
  y2RotationController = new ControllerInt(*y2Spinbox,*y2Slider,*clipRotYWrapper);
  z2RotationController = new ControllerInt(*z2Spinbox,*z2Slider,*clipRotZWrapper);
}

void ClippingLayout::buildTuples()
{
  salmonMagentaTup = std::make_tuple(x1SalmonController,x1MagentaController,x1MSlider);
  greenYellowTup   = std::make_tuple(y1GreenController,y1YellowController,y1YSlider);
  purpleTealTup    = std::make_tuple(z1PurpleController,z1TealController,z1TSlider);
}

std::tuple<int,int> ClippingLayout::sliderGap(int halfVal, int size)
{
  return std::make_tuple(halfVal - size/2, halfVal + size/2);
}

/*
* This function sets the mins and max of each controller that isn't the value affected.
*/
void ClippingLayout::resetValues(ColorTup c1, ColorTup c2)
{
  std::get<0>(c1)->setValues(0);
  std::get<1>(c1)->setValues(std::get<2>(c1)->maximum());
  std::get<0>(c2)->setValues(0);
  std::get<1>(c2)->setValues(std::get<2>(c2)->maximum());
}

void ClippingLayout::disableToolChains()
{
  for(const auto &toolButton : ToolButtons)
    toolButton->setChecked(false);
}

// I disable the toolbuttons before. It's kind of a nasty trick that I am not sure 
// how it will affect performance. 
void ClippingLayout::onYZButtonPushed()
{
  disableToolChains();
  auto[lSliderVal,rSliderVal] = sliderGap(HALFXY,setClipSpinbox->value());
  setSalmonValue(lSliderVal);
  setMagentaValue(rSliderVal);
  resetValues(greenYellowTup,purpleTealTup);
  xChainToolbutton->setChecked(true);
}

void ClippingLayout::onXZButtonPushed()
{
  disableToolChains();
  auto[lSliderVal,rSliderVal] = sliderGap(HALFXY,slabSpinbox->value());
  setGreenValue(lSliderVal);
  setYellowValue(rSliderVal);
  resetValues(salmonMagentaTup,purpleTealTup);
  yChainToolbutton->setChecked(true);
}

void ClippingLayout::onXYButtonPushed()
{
  disableToolChains();
  auto[lSliderVal,rSliderVal] = sliderGap(HALFZ,widthSpinbox->value());
  setPurpleValue(lSliderVal);
  setTealValue(rSliderVal);
  resetValues(salmonMagentaTup,greenYellowTup);
  zChainToolbutton->setChecked(true);
}

void ClippingLayout::setSalmonValue(int newVal)
{
  if (xChainToolbutton->isChecked())
  {
    const int distance = x1MagentaController->getValue() - x1SalmonController->getValue();
    x1SSlider->setUpperBoundValue(x1SSlider->maximum()-distance);
    x1SalmonController->setValues(newVal);
    x1MagentaController->setValues(x1SalmonController->getValue()+distance);
  }
  else
  { 
    x1SSlider->setUpperBoundValue(x1MagentaController->getValue());
    x1SalmonController->setValues(newVal);
  }
}

void ClippingLayout::setMagentaValue(int newVal)
{
  if (xChainToolbutton->isChecked())
  {
    const int distance = x1MagentaController->getValue() - x1SalmonController->getValue();
    x1MSlider->setLowerBoundValue(distance);
    x1MagentaController->setValues(newVal);
    x1SalmonController->setValues(x1MagentaController->getValue()-distance);
  }
  else
  { 
    x1MSlider->setLowerBoundValue(x1SalmonController->getValue());
    x1MagentaController->setValues(newVal);
  }
}

void ClippingLayout::setGreenValue(int newVal)
{
  if (yChainToolbutton->isChecked())
  {
    const int distance = y1YellowController->getValue() - y1GreenController->getValue();
    y1GSlider->setUpperBoundValue(y1GSlider->maximum()-distance);
    y1GreenController->setValues(newVal);
    y1YellowController->setValues(y1GreenController->getValue()+distance);
  }
  else
  {
    y1GSlider->setUpperBoundValue(y1YellowController->getValue());
    y1GreenController->setValues(newVal);
  }
}

void ClippingLayout::setYellowValue(int newVal)
{
  if (yChainToolbutton->isChecked())
  {
    const int distance = y1YellowController->getValue() - y1GreenController->getValue();
    y1YSlider->setLowerBoundValue(distance);
    y1YellowController->setValues(newVal);
    y1GreenController->setValues(y1YellowController->getValue() - distance);
  }
  else
  {
    y1YSlider->setLowerBoundValue(y1GreenController->getValue());
    y1YellowController->setValues(newVal);
  }

}

void ClippingLayout::setPurpleValue(int newVal)
{
  if (zChainToolbutton->isChecked())
  {
    const int distance = z1TealController->getValue() - z1PurpleController->getValue();
    z1PSlider->setUpperBoundValue(z1PSlider->maximum()-distance);
    z1PurpleController->setValues(newVal);
    z1TealController->setValues(z1PurpleController->getValue() + distance);
  }
  else
  {
    z1PSlider->setUpperBoundValue(z1TealController->getValue());
    z1PurpleController->setValues(newVal);
  }
}

void ClippingLayout::setTealValue(int newVal)
{
  if (zChainToolbutton->isChecked())
  {
    const int distance = z1TealController->getValue() - z1PurpleController->getValue();
    z1TSlider->setLowerBoundValue(distance);
    z1TealController->setValues(newVal);
    z1PurpleController->setValues(z1TealController->getValue() - distance);
  }
  else
  {
    z1TSlider->setLowerBoundValue(z1PurpleController->getValue());
    z1TealController->setValues(newVal);
  }
}
