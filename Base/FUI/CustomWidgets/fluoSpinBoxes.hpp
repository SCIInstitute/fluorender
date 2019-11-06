#ifndef FLUO_SPIN_BOXES_HPP
#define FLUO_SPIN_BOXES_HPP

#include <QObject>
#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>

template<typename SpinBoxType, typename T = int>
class FluoSpinBox : public SpinBoxType
{
  Q_OBJECT

  public:
    FluoSpinBox(QWidget* parent = nullptr,T floor, T ceiling, bool hasButtons = true) : SpinBoxType(parent)
    {
      this->setRange(floor,ceiling); 

      if(!hasButtons)
        this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    }
    
    void updateValue(T value) { this->setValue(value); }

    T get() const { return this->value(); }
};

#endif
