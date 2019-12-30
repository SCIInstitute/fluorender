#include "autoKeyPage.hpp"

AutoKeyPage::AutoKeyPage()
{
  setTableContents();
  addRow0();
  addRow1();
  addRow2();
  this->setLayout(gridLayout);
}

void AutoKeyPage::setTableContents()
{
  table->setHorizontalHeaderLabels(headers);

  for(int i = 0; i < table->rowCount(); ++i)
    table->setItem(i,0,tableContents.at(i));
}

void AutoKeyPage::addRow0()
{
  gridLayout->addWidget(autoKeyLabel,0,0,0,4);
}

void AutoKeyPage::addRow1()
{
  gridLayout->addWidget(table,1,0,0,4);
}

void AutoKeyPage::addRow2()
{
  gridLayout->addWidget(generatePushButton,2,4);
}
