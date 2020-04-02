#include "clippingPlane.hpp"

ClippingPlane::ClippingPlane()
{
  outputFrame->setLayout(clippingLayout);
  frameLayout->addWidget(outputFrame);
  this->setLayout(frameLayout);

  makeStdAnyConnections();
  makeIntConnections();

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
