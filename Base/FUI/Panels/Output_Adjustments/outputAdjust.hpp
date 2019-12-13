#ifndef OUTPUT_ADJUSTMENTS_HPP
#define OUTPUT_ADJUSTMENTS_HPP

#include <QWidget>

#include "colorPanel.hpp"


class OutputAdjustments : public QWidget
{
  Q_OBJECT
  
  public:
    OutputAdjustments();

  private:

    void setWidgetLayers();

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();

    QGridLayout *outputLayout = new QGridLayout();

    QLabel *gammaLabel = new QLabel("Gam.");
    QLabel *luminLabel = new QLabel("Lum.");
    QLabel *eqlLabel = new QLabel("Eql.");

    QWidget *redWidget = new QWidget();
    QWidget *greenWidget = new QWidget();
    QWidget *blueWidget = new QWidget();

    ColorPanel *redPanel = new ColorPanel("Red");
    ColorPanel *greenPanel = new ColorPanel("Green");
    ColorPanel *bluePanel = new ColorPanel("Blue");

    QPushButton *setDefaultButton = new QPushButton();

};

#endif
