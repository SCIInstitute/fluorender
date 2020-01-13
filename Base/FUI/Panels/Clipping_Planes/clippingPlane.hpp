#ifndef CLIPPING_PLANE_HPP
#define CLIPPING_PLANE_HPP

#include <QWidget>
#include <QGroupBox>

#include "clippingLayout.hpp"

class ClippingPlane : public QWidget
{
  Q_OBJECT

  public: 
    ClippingPlane()
    {
      outputFrame->setLayout(clippingLayout);
      frameLayout->addWidget(outputFrame);
      this->setLayout(frameLayout);
    }

  private:
    QGroupBox *outputFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    ClippingLayout *clippingLayout = new ClippingLayout();
};



#endif
