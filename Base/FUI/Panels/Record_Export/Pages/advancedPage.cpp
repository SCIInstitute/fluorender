#include "advancedPage.hpp"

AdvancedPage::AdvancedPage()
{
  setDropDown();
  setTableWidgetContents();
  addRow0();
  addRow1();
  addRow2();
  this->setLayout(gridLayout);
}

void AdvancedPage::setDropDown()
{
  dropDown->addItems(dropText);
}

void AdvancedPage::setTableWidgetContents()
{
  table->setHorizontalHeaderLabels(tableLabels);
}

void AdvancedPage::addRow0()
{
  gridLayout->addWidget(keyFramesLabel,0,0);
}

void AdvancedPage::addRow1()
{
  gridLayout->addWidget(table,1,0,1,6);
}

void AdvancedPage::addRow2()
{
  gridLayout->addWidget(defaultLabel,2,0);
  gridLayout->addWidget(fpsSpinbox,2,1);
  gridLayout->addWidget(dropDown,2,2);
  gridLayout->addWidget(addButton,2,3);
  gridLayout->addWidget(deleteButton,2,4);
  gridLayout->addWidget(deleteAllButton,2,5);
}
