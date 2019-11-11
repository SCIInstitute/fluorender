#ifndef FLUO_SLIDER_HPP
#define FLUO_SLIDER_HPP

#include <QWidget>
#include <QSlider>

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
    void updateValue(int value) { this->setValue(value); }
    int get() const { return this->value(); }
};

#endif
