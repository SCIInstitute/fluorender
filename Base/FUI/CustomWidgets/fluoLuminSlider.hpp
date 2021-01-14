#ifndef FLUO_LUMIN_SLIDER_HPP
#define FLUO_LUMIN_SLIDER_HPP

#include <QWidget>
#include <QSlider>

#include "fluoSliderStyle.hpp"

class FluoLuminSlider : public QSlider
{
  Q_OBJECT

	public:

      FluoLuminSlider(Qt::Orientation ori, int floor, int ceiling)
	  {
        this->setStyle(new MyStyle(this->style()));
		this->setOrientation(ori);
		this->setRange(floor,ceiling);
	  }

	  void updateValue(double value)
	  {
        this->setValue(floatToInt(value)); 
	  }

	  const double getValueAsDouble()
	  {
        return this->value()/100.0;
	  }

	private:
	  const int floatToInt(double value) { return static_cast<int>(value * 100.0 + 0.5); }

};



#endif
