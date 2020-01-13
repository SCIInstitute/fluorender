#ifndef FLUO_LINE_HPP
#define FLUO_LINE_HPP

#include <QFrame>

class FluoLine : public QFrame
{
  Q_OBJECT

  public:
    FluoLine(const QFrame::Shape &shape = QFrame::HLine) : frameShape(shape)
    {
      this->setFrameShape(frameShape);
      this->setFrameShadow(QFrame::Sunken);
    }

  private:
    QFrame::Shape frameShape;
};


#endif
