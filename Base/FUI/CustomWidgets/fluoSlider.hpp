#ifndef FLUO_SLIDER_HPP
#define FLUO_SLIDER_HPP

#include <QWidget>
#include <QSlider>

#include <type_traits>
#include <any>

#include "fluoSliderStyle.hpp"

class FluoSlider : public QSlider
{
  Q_OBJECT

  public:

    FluoSlider(Qt::Orientation ori, int floor, int ceiling)
    {
        this->setStyle(new MyStyle(this->style()));
        this->setOrientation(ori);
        this->setRange(floor,ceiling);
    }

    void updateValue(std::any value)
    {
      try
      {
        this->setValue(std::any_cast<int>(value));
      }
      catch (const std::bad_any_cast &e)
      {
        (void)e;
        this->setValue(static_cast<int>(std::any_cast<double>(value) * 100.0 + 0.5));
      }
    }

    int get() const { return this->value(); }
};

#endif
