#include "croppingPage.hpp"

CroppingPage::CroppingPage()
{
  addRow0();
  addRow1();
  addRow2();
  this->setLayout(gridLayout);
}

void CroppingPage::addRow0()
{
  gridLayout->addWidget(enableCroppingCheckbox,0,0,0,2);
  gridLayout->addWidget(enableCroppingCheckbox,0,3);
}

void CroppingPage::addRow1()
{
  gridLayout->addWidget(centerLabel,1,0);
  gridLayout->addWidget(xLabel,1,1);
  gridLayout->addWidget(xSpinbox,1,2);
  gridLayout->addWidget(yLabel,1,3);
  gridLayout->addWidget(ySpinbox,1,4);
}

void CroppingPage::addRow2()
{
  gridLayout->addWidget(sizeLabel,2,0);
  gridLayout->addWidget(widthLabel,2,1);
  gridLayout->addWidget(widthSpinbox,2,2);
  gridLayout->addWidget(heightLabel,2,3);
  gridLayout->addWidget(heightSpinbox,2,4);
}
