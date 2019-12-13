#include "colorPanel.hpp"

ColorPanel::ColorPanel(std::string_view col) : color(col)
{
  addRow0();
  addRow1();
  addRow2();
  addRow3();
  addRow4();
}

void ColorPanel::addRow0()
{
  this->addWidget(colorLabel,0,0);
  this->addWidget(sepLine,0,1,Qt::AlignCenter);
  this->addWidget(chainButton,0,2);
}

void ColorPanel::addRow1()
{
  this->addWidget(colorLine,1,0,1,2);
}

void ColorPanel::addRow2()
{
  this->addWidget(gammaSlider,2,0,Qt::AlignCenter);
  this->addWidget(luminSlider,2,1,Qt::AlignCenter);
  this->addWidget(eqlSlider,2,2,Qt::AlignCenter);
}

void ColorPanel::addRow3()
{
  this->addWidget(resetButton,3,0,1,2);
}

void ColorPanel::addRow4()
{
  this->addWidget(gammaSpinbox,4,0);
  this->addWidget(luminSpinbox,4,1);
  this->addWidget(eqlSpinbox,4,2);
}
