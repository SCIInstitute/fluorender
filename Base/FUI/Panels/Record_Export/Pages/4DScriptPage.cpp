#include "4DScriptPage.hpp"

FourDScriptPage::FourDScriptPage()
{
  setTableContents();
  constructPage();
  this->setLayout(gridLayout);
}

void FourDScriptPage::setTableContents()
{
  table->setHorizontalHeaderLabels(header);

  for(int i = 0; i < table->rowCount(); ++i)
    table->setItem(i,0,rowItems.at(i));

}

void FourDScriptPage::addRow0()
{
  gridLayout->addWidget(enableCheckbox,0,0,0,2);
}

void FourDScriptPage::addRow1()
{
  gridLayout->addWidget(enableDescLabel,1,0,0,2);
}

void FourDScriptPage::addRow2()
{
  gridLayout->addWidget(scriptLabel,2,0);
  gridLayout->addWidget(browseButton,2,1);
}

void FourDScriptPage::addRow3()
{
  gridLayout->addWidget(fileLocTextbar,3,0);
  gridLayout->addWidget(deleteToolbutton,3,1);
}

void FourDScriptPage::addRow4()
{
  gridLayout->addWidget(table,4,0,0,2);
}

void FourDScriptPage::constructPage()
{
  for(const auto& fn : rows)
    fn();
}
