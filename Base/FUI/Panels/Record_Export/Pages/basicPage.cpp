#include "basicPage.hpp"

BasicPage::BasicPage()
{
  setButtonGroup();
  setComboBox();
  constructLayout();
  this->setLayout(gridLayout);
}

void BasicPage::setComboBox()
{
  styleComboBox->addItems(styleList);
}

void BasicPage::setButtonGroup()
{
  buttonGroup->addButton(xButton);
  buttonGroup->addButton(yButton);
  buttonGroup->addButton(zButton);
}

void BasicPage::addRow0()
{
  gridLayout->addWidget(rotationCheckBox,0,0,1,2);
}

void BasicPage::addRow1()
{
  gridLayout->addWidget(xButton,1,0);
}

void BasicPage::addRow2()
{
  gridLayout->addWidget(yButton,2,0);
  gridLayout->addWidget(degreesSpinbox,2,1);
  gridLayout->addWidget(degreesLabel,2,2);
  gridLayout->addWidget(styleLabel,2,4);
  gridLayout->addWidget(styleComboBox,2,5,1,2);
}

void BasicPage::addRow3()
{
  gridLayout->addWidget(zButton,3,0);
}

void BasicPage::addRow4()
{
  gridLayout->addWidget(timeBatchCheckBox,4,0,1,3);
  gridLayout->addWidget(currentTimeLabel,4,5,1,2);
}

void BasicPage::addRow5()
{
  gridLayout->addWidget(startTimeLabel,5,0);
  gridLayout->addWidget(startTimeSpinbox,5,1);
  gridLayout->addWidget(endTimeLabel,5,2);
  gridLayout->addWidget(endTimeSpinbox,5,3);
  gridLayout->addWidget(minusPushbutton,5,5);
  gridLayout->addWidget(currentTimeSpinbox,5,6);
  gridLayout->addWidget(plusPushbutton,5,7);
}

void BasicPage::addRow6()
{
  gridLayout->addWidget(movieLenLabel,6,3,1,2);
  gridLayout->addWidget(secondsSpinbox,6,6);
  gridLayout->addWidget(secondsLabel,6,7);
}

void BasicPage::constructLayout()
{
  for(const auto &fn : rowFuncs)
    fn();
}
