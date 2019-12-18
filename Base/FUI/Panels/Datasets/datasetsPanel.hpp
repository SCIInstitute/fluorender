#ifndef DATASETS_PANEL_HPP
#define DATASETS_PANEL_HPP

#include <QWidget>
#include <QGroupBox>

#include "datasetsLayout.hpp"

class DatasetsPanel : public QWidget
{
  Q_OBJECT

  public:
    DatasetsPanel()
    {
      datasetsFrame->setLayout(datasetsLayout);
      frameLayout->addWidget(datasetsFrame);
      this->setLayout(frameLayout);
    }

  private:
    QGroupBox *datasetsFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    DatasetsLayout *datasetsLayout = new DatasetsLayout();
};

#endif
