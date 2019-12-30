#include "recordExportLayout.hpp"

RecordExportLayout::RecordExportLayout()
{
  constructPages();
  addRow0();
  addRow1();
  addRow2();
}

void RecordExportLayout::constructPages()
{
  for(const auto &page : pages)
    tabWidget->addTab(page.first,page.second);
}

void RecordExportLayout::addRow0()
{
  this->addWidget(tabWidget,0,0,0,7);
}

void RecordExportLayout::addRow1()
{
  this->addWidget(fpsLabel,2,0);
  this->addWidget(fpsSpinbox,2,1);
  this->addWidget(captureLabel,2,3);
  this->addWidget(captureCombobox,2,4,0,2);
  this->addWidget(helpToolbutton,2,7);
}

void RecordExportLayout::addRow2()
{
  this->addWidget(playToolbutton,3,0);
  this->addWidget(backToolbutton,3,1);
  this->addWidget(playBackSlider,3,2);
  this->addWidget(secondsSpinbox,3,4);
  this->addWidget(secondsLabel,3,5);
  this->addWidget(savePushbutton,3,6);
}
