#include "outputAdjust.hpp"

OutputAdjustments::OutputAdjustments()
{
  setWidgetLayers();

  addRow0();
  addRow1();
  addRow2();
  addRow3();
  addRow4();

  this->setLayout(outputLayout);
}

void OutputAdjustments::setWidgetLayers()
{
  redWidget->setLayout(redPanel);
  greenWidget->setLayout(greenPanel);
  blueWidget->setLayout(bluePanel);
}

void OutputAdjustments::addRow0()
{
  outputLayout->addWidget(gammaLabel,0,0);
  outputLayout->addWidget(luminLabel,0,1);
  outputLayout->addWidget(eqlLabel,0,2);
}

void OutputAdjustments::addRow1()
{
  outputLayout->addWidget(redWidget,1,0,1,2);
}

void OutputAdjustments::addRow2()
{
  outputLayout->addWidget(greenWidget,2,0,1,2);
}

void OutputAdjustments::addRow3()
{
  outputLayout->addWidget(blueWidget,3,0,1,2);
}
void OutputAdjustments::addRow4()
{
  outputLayout->addWidget(setDefaultButton,4,0,1,2);
}
