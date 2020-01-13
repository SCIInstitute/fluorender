#include "recordTabLayout.hpp"

RecordTabLayout::RecordTabLayout()
{
  constructPages();
  this->addWidget(tabWidget);
}

void RecordTabLayout::constructPages()
{
  for(const auto &page : pages)
    tabWidget->addTab(page.first,page.second);
}
