#ifndef COLOR_PANEL_HPP
#define COLOR_PANEL_HPP

#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QGridLayout>
#include <QString>

#include <CustomWidgets/fluoLine.hpp>
#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoColoredLine.hpp>

class ColorPanel : public QGridLayout
{
  Q_OBJECT

  public:
    ColorPanel(std::string_view col);

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();

  private:
    std::string_view color; 

    QLabel *colorLabel = new QLabel(QString::fromStdString(std::string(color)));

    FluoLine *sepLine = new FluoLine(QFrame::VLine);
    FluoColoredLine *colorLine = new FluoColoredLine(QFrame::HLine,color);

    QToolButton *chainButton = new QToolButton();

    FluoSlider *gammaSlider = new FluoSlider(Qt::Vertical,10,400);
    FluoSlider *luminSlider = new FluoSlider(Qt::Vertical,-256,256);
    FluoSlider *eqlSlider   = new FluoSlider(Qt::Vertical,0,100);

    FluoSpinboxDouble *gammaSpinbox = new FluoSpinboxDouble(0.10,4.0,false);
    FluoSpinbox *luminSpinbox = new FluoSpinbox(-256,256,false);
    FluoSpinboxDouble *eqlSpinbox = new FluoSpinboxDouble(0.0,1.0,false);

    QPushButton *resetButton = new QPushButton();

};

#endif
