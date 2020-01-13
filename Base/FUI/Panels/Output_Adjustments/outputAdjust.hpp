#ifndef OUTPUT_ADJUSTMENTS_HPP
#define OUTPUT_ADJUSTMENTS_HPP

#include <QWidget>
#include <QGroupBox>

#include "outputLayout.hpp"

class OutputAdjustments : public QWidget
{
  Q_OBJECT
  
  public:
    OutputAdjustments()
    {
      outputFrame->setLayout(outputLayout);
      frameLayout->addWidget(outputFrame);
      this->setLayout(frameLayout);
    }

  private:

    QGroupBox *outputFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    OutputLayout *outputLayout = new OutputLayout();


};

#endif
