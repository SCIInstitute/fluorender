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
