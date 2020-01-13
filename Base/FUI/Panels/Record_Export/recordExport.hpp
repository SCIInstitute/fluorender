#ifndef RECORD_EXPORT_HPP
#define RECORD_EXPORT_HPP

#include <QWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QFrame>

#include "recordTabLayout.hpp"
#include "recordPlayLayout.hpp"

class RecordExport : public QWidget
{
  Q_OBJECT

  public:

    RecordExport()
    {
      topFrame->setLayout(tabLayout);
      botFrame->setLayout(playLayout);
      frameLayout->addWidget(topFrame,0,0);
      frameLayout->addWidget(botFrame,1,0);
      recordExportFrame->setLayout(frameLayout);
      widgetLayout->addWidget(recordExportFrame);
      this->setLayout(widgetLayout);
    }

  private:

    QGroupBox *recordExportFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    QGridLayout *widgetLayout = new QGridLayout();

    QFrame *topFrame = new QFrame();
    QFrame *botFrame = new QFrame();

    RecordTabLayout *tabLayout = new RecordTabLayout();
    RecordPlayLayout *playLayout = new RecordPlayLayout();

};


#endif
