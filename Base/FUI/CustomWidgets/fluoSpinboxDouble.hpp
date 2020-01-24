#ifndef FLUO_SPINBOX_DOUBLE_HPP
#define FLUO_SPINBOX_DOUBLE_HPP

#include <QWidget>
#include <QString>
#include <QDoubleSpinBox>

#include <any>

// TODO: Fix this code so it will update the value based on input

class FluoSpinboxDouble : public QDoubleSpinBox
{
  Q_OBJECT

  public:
    using QDoubleSpinBox::QDoubleSpinBox;

    FluoSpinboxDouble(double floor, double ceiling, bool hasButtons)
    {
      this->setRange(floor,ceiling);

      if(!hasButtons)
        this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    }
   /*
    template<typename T>
    void updateValue(T value)
    {
      if(std::is_same_v<T,double>)
        this->setValue(value);
      else
        this->setValue(static_cast<double>(value / 100.0));
    }
    */

    void updateValue(std::any value)
    {
      try
      {
        this->setValue(std::any_cast<double>(value));
      }
      catch (const std::bad_any_cast &e)
      {
        this->setValue(static_cast<double>(std::any_cast<int>(value) / 100.0));
      }
    }

    double get() const { return this->value(); }

  protected:
    void fixup(QString &input) const
    {
      auto isOk = false;
      double value = input.toDouble(&isOk);

      if(isOk)
      {
        value = qBound(minimum(),value,maximum());
        input = QString::number(value);
      }
      else
      {
        QDoubleSpinBox::fixup(input);
      }
    }

    QValidator::State validate(QString &text, int &pos) const
    {
      auto isOk = false;
      double value = text.toDouble(&isOk);

      if(isOk)
      {
        if(value >= minimum() && value <= maximum())
          return QValidator::Acceptable;
        return QValidator::Intermediate;
      }
      else
      {
          return QDoubleSpinBox::validate(text,pos);
      }
    }
};

#endif
