#ifndef FLUO_SPINBOX_HPP
#define FLUO_SPINBOX_HPP

#include <QWidget>
#include <QSpinBox>

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
    
    template<typename T>
    void updateValue(T value)
    {
      if(std::is_same_v<T,int>)
        this->setValue(value);
      else
        this->setValue(static_cast<int>(value * 100.0 + 0.5));
    }

    int get() const { return this->value(); }
};

#endif
