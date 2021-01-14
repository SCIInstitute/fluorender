#ifndef FLUO_LUMIN_SPIN_HPP
#define FLUO_LUMIN_SPIN_HPP

#include <QWidget>
#include <QSpinBox>


class FluoLuminSpinbox : public QSpinBox
{
  Q_OBJECT

	public:
    FluoLuminSpinbox(int floor,int ceiling, bool hasButtons)
	{
      this->setRange(floor,ceiling);

	  if(!hasButtons)
	    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
	}

	void updateValue(double value)
	{
      this->setValue(luminConversion(value));      
	}

	const double getRawDouble()
	{
	  return rawConversion(this->value());
	}


	private:
	  const int luminConversion(double value) { return static_cast<int>((value-1.0)*256.0 + 0.5); }
      const double rawConversion(int value) { return static_cast<double>((value/256.0) + 1.0); }

};



#endif
