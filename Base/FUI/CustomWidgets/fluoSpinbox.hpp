#ifndef FLUO_SPINBOX_HPP
#define FLUO_SPINBOX_HPP

#include <QWidget>
#include <QSpinBox>

#include <any>

class FluoSpinbox : public QSpinBox
{
  Q_OBJECT

  public:
    FluoSpinbox(int floor, int ceiling, bool hasButtons)
    {
      this->setRange(floor,ceiling);

      if(!hasButtons)
        this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    }

    void updateValue(int value)
    {
      this->setValue(value);
    }

    void updateValue(double value)
    {
      this->setValue(static_cast<int>(value * 100.0 + 0.5));
    }

    int get() const { return this->value(); }
};

#endif
