#ifndef RECORD_EXPORT_HPP
#define RECORD_EXPORT_HPP

#include <QWidget>
#include <QGroupBox>

#include "recordExportLayout.hpp"

class RecordExport : public QWidget
{
  Q_OBJECT

  public:

    RecordExport()
    {
      recordExportFrame->setLayout(recordExportLayout);
      frameLayout->addWidget(recordExportFrame);
      this->setLayout(frameLayout);
    }

  private:

    QGroupBox *recordExportFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    RecordExportLayout *recordExportLayout = new RecordExportLayout();

};


#endif
