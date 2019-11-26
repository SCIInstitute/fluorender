#ifndef FLUO_SPINBOX_DOUBLE_HPP
#define FLUO_SPINBOX_DOUBLE_HPP

#include <QWidget>
#include <QDoubleSpinBox>

class FluoSpinboxDouble : public QDoubleSpinBox
{
  Q_OBJECT

  public:
    FluoSpinboxDouble(double floor, double ceiling, bool hasButtons)
    {
      this->setRange(floor,ceiling);

      if(!hasButtons)
        this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    }
    
    void updateValue(double value) { this->setValue(value); }

    double get() const { return this->value(); }
};

#endif
