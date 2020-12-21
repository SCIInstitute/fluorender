#ifndef FLUO_SLIDER_HPP
#define FLUO_SLIDER_HPP

#include <QWidget>
#include <QSlider>

#include <type_traits>

#include "fluoSliderStyle.hpp"

class FluoSlider : public QSlider
{
  Q_OBJECT

  public:

    FluoSlider(Qt::Orientation ori, int floor, int ceiling)
    {
        connect(this, SIGNAL(valueChanged(int)),this,SLOT(restrictLowerBoundMove(int)));
        connect(this, SIGNAL(valueChanged(int)),this,SLOT(restrictUpperBoundMove(int)));
        this->setStyle(new MyStyle(this->style()));
        this->setOrientation(ori);
        this->setRange(floor,ceiling);
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

    void setLowerBoundValue(int value)
    {
      lowerBoundRestriction = value; 
    }

    void setUpperBoundValue(int value)
    {
      upperBoundRestriction = value;
    }

    void resetDefaults()
    {
      lowerBoundRestriction = DEFAULT_LOWER;
      upperBoundRestriction = DEFAULT_UPPER;
    }

  private slots:
    void restrictLowerBoundMove(int index)
    {
      if(index < lowerBoundRestriction)
        this->setSliderPosition(lowerBoundRestriction);
    }
   
    void restrictUpperBoundMove(int index)
    {
      if(index > upperBoundRestriction)
        this->setSliderPosition(upperBoundRestriction);
    }
  
  private:
    const int DEFAULT_LOWER = -200000;
    const int DEFAULT_UPPER =  200000;
    int lowerBoundRestriction = DEFAULT_LOWER;
    int upperBoundRestriction = DEFAULT_UPPER;
};

#endif
