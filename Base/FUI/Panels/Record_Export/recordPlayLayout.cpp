#include "recordPlayLayout.hpp"

RecordPlayLayout::RecordPlayLayout()
{
  loadCombobox();
  addRow0();
  addRow1();
}

void RecordPlayLayout::loadCombobox()
{
  captureCombobox->addItem("Renderview 0");
}

void RecordPlayLayout::addRow0()
{
  this->addWidget(fpsLabel,2,0);
  this->addWidget(fpsSpinbox,2,1);
  this->addWidget(captureLabel,2,3);
  this->addWidget(captureCombobox,2,4,1,2);
  this->addWidget(helpToolbutton,2,7);
}

void RecordPlayLayout::addRow1()
{
  this->addWidget(playToolbutton,3,0);
  this->addWidget(backToolbutton,3,1);
  this->addWidget(playBackSlider,3,2);
  this->addWidget(secondsSpinbox,3,3);
  this->addWidget(secondsLabel,3,4);
  this->addWidget(savePushbutton,3,5,1,3);
}
