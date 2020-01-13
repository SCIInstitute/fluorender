#ifndef OUTPUT_LAYOUT_HPP
#define OUTPUT_LAYOUT_HPP

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QString>

#include <CustomWidgets/fluoLine.hpp>
#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoColoredLine.hpp>
#include <CustomWidgets/fluoToolButton.hpp>

#include <cmath>

class OutputLayout : public QGridLayout
{
  Q_OBJECT

  public:
   OutputLayout();

  private:

   const int MAXCOLSIZE = 2;

   template<typename T>
   void addToWidget(T* widget,int row,int size)
   {
     int col = std::abs(size - MAXCOLSIZE);
     this->addWidget(widget,row,col,Qt::AlignHCenter);
   }

   template<typename T>
   void addRow(int row, T* widget)
   {
     this->addWidget(widget,row,MAXCOLSIZE,Qt::AlignHCenter);
   }

   template<typename Widget, typename... T>
   void addRow(int row, Widget *w,T*...t)
   {
     constexpr std::size_t n = sizeof...(T);
     addToWidget(w,row,n);
     addRow(row,t...);
   }

   template<typename T>
   void addSingle(int row, T *widget)
   {
     this->addWidget(widget,row,0,1,MAXCOLSIZE+1,Qt::AlignVCenter);
   }


   QLabel *gammaLabel     = new QLabel("Gamma");
   QLabel *luminanceLabel = new QLabel("Luminance");
   QLabel *equalLabel     = new QLabel ("Equalization");
   QLabel *redLabel       = new QLabel("Red");
   QLabel *greenLabel     = new QLabel("Green");
   QLabel *blueLabel      = new QLabel("Blue");

   FluoLine *redSepLine     = new FluoLine(QFrame::VLine);
   FluoLine *greenSepLine   = new FluoLine(QFrame::VLine);
   FluoLine *blueSepLine    = new FluoLine(QFrame::VLine);
   FluoLine *setDefaultLine = new FluoLine(QFrame::HLine);

   FluoToolButton *redChainButton   = new FluoToolButton(":/output-unlink.svg","",true,true,false);
   FluoToolButton *greenChainButton = new FluoToolButton(":/output-unlink.svg","",true,true,false);
   FluoToolButton *blueChainButton  = new FluoToolButton(":/output-unlink.svg","",true,true,false);

   QPushButton *redResetButton   = new QPushButton(QIcon(":/output-reset.svg")," Reset");
   QPushButton *greenResetButton = new QPushButton(QIcon(":/output-reset.svg")," Reset");
   QPushButton *blueResetButton  = new QPushButton(QIcon(":/output-reset.svg")," Reset");
   QPushButton *resetAllButton   = new QPushButton(QIcon(":/output-reset.svg")," Reset All");

   FluoColoredLine *redLine   = new FluoColoredLine(QFrame::HLine,"Red");
   FluoColoredLine *greenLine = new FluoColoredLine(QFrame::HLine,"Green");
   FluoColoredLine *blueLine  = new FluoColoredLine(QFrame::HLine,"Blue");

   FluoSlider *redGammaSlider   = new FluoSlider(Qt::Vertical,10,400);
   FluoSlider *greenGammaSlider = new FluoSlider(Qt::Vertical,10,400);
   FluoSlider *blueGammaSlider  = new FluoSlider(Qt::Vertical,10,400);

   FluoSlider *redLuminSlider   = new FluoSlider(Qt::Vertical,-256,256);
   FluoSlider *greenLuminSlider = new FluoSlider(Qt::Vertical,-256,256);
   FluoSlider *blueLuminSlider  = new FluoSlider(Qt::Vertical,-256,256);

   FluoSlider *redEqlSlider   = new FluoSlider(Qt::Vertical,0,100);
   FluoSlider *greenEqlSlider = new FluoSlider(Qt::Vertical,0,100);
   FluoSlider *blueEqlSlider  = new FluoSlider(Qt::Vertical,0,100);

   FluoSpinboxDouble *redGammaSpinbox   = new FluoSpinboxDouble(0.10,4.0,false);
   FluoSpinboxDouble *greenGammaSpinbox = new FluoSpinboxDouble(0.10,4.0,false);
   FluoSpinboxDouble *blueGammaSpinbox  = new FluoSpinboxDouble(0.10,4.0,false);

   FluoSpinbox *redLuminSpinbox   = new FluoSpinbox(-256,256,false);
   FluoSpinbox *greenLuminSpinbox = new FluoSpinbox(-256,256,false);
   FluoSpinbox *blueLuminSpinbox  = new FluoSpinbox(-256,256,false);

   FluoSpinboxDouble *redEqlSpinbox   = new FluoSpinboxDouble(0.0,1.0,false);
   FluoSpinboxDouble *greenEqlSpinbox = new FluoSpinboxDouble(0.0,1.0,false);
   FluoSpinboxDouble *blueEqlSpinbox  = new FluoSpinboxDouble(0.0,1.0,false);
};

#endif 
