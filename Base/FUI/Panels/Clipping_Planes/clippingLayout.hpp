#ifndef CLIPPING_LAYOUT_HPP
#define CLIPPING_LAYOUT_HPP

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
#include <CustomWidgets/fluoToolButton.hpp>

#include <cmath>

class ClippingLayout : public QGridLayout
{
  Q_OBJECT

  public:
    ClippingLayout()
    {
      row0();
      addSingle(1,lineSep1);
      addThreeToRowShift(2,x1Label,y1Label,z1Label);
      addThreeToRow(3,x1SSpinbox,y1GSpinbox,z1PSpinbox);
      addThreeToRow(4,salmonHLine,greenHLine,purpleHLine);
      addRow(5,x1SSlider,salmonVLine,x1MSlider,y1GSlider,greenVLine,y1YSlider,z1PSlider,purpleVLine,z1TSlider);
      addThreeToRow(6,magentaHLine,yellowHLine,tealHLine);
      addThreeToRowShift(7,xChainToolbutton,yChainToolbutton,zChainToolbutton);
      addSingle(8,lineSep2);
      addThreeToRowShift(9,setClipLabel,slabLabel,widthLabel);
      addThreeToRow(10,yzButton,xzButton,xyButton);
      addThreeToRow(11,setClipSpinbox,slabSpinbox,widthSpinbox);
      addSingle(12,resetClipsButton);
      addSingle(13,lineSep3);
      addSingle(14,rotationsLabel);
      addSingle(15,alignToViewButton);
      addSingle(16,resetTo0Button);
      addThreeToRowShift(17,x2Label,y2Label,z2Label);
      addThreeToRow(18,z2Spinbox,y2Spinbox,x2Spinbox);
      addThreeToRowShift(19,z2Slider,y2Slider,x2Slider);
    }

  private:

   const int MAXCOLSIZE = 8;


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
   void addThreeToRow(int row, T *w1, T *w2, T *w3)
   {
     this->addWidget(w1,row,0,1,3);
     this->addWidget(w2,row,3,1,3);
     this->addWidget(w3,row,6,1,3);
   }

   template<typename T>
   void addThreeToRowShift(int row, T *w1, T *w2, T *w3)
   {
     this->addWidget(w1,row,1,1,3);
     this->addWidget(w2,row,4,1,3);
     this->addWidget(w3,row,7,1,2);
   }

   template<typename T>
   void addSingle(int row, T *widget)
   {
     this->addWidget(widget,row,0,1,MAXCOLSIZE+1,Qt::AlignVCenter);
   }


   QToolButton *linkToolButton   = new QToolButton();
   QToolButton *alwaysToolButton = new QToolButton();
   QToolButton *toggleToolButton = new QToolButton();

   FluoToolButton *xChainToolbutton = new FluoToolButton(":/clipping-unlink.svg","",true,true,false);
   FluoToolButton *yChainToolbutton = new FluoToolButton(":/clipping-unlink.svg","",true,true,false);
   FluoToolButton *zChainToolbutton = new FluoToolButton(":/clipping-unlink.svg","",true,true,false);

   FluoLine *lineSep1 = new FluoLine();
   FluoLine *lineSep2 = new FluoLine();
   FluoLine *lineSep3 = new FluoLine();

   QLabel *x1Label        = new QLabel("X");
   QLabel *y1Label        = new QLabel("Y");
   QLabel *z1Label        = new QLabel("Z");
   QLabel *setClipLabel   = new QLabel("Set Clip");
   QLabel *slabLabel      = new QLabel("Slab");
   QLabel *widthLabel     = new QLabel("Width");
   QLabel *rotationsLabel = new QLabel("Rotations:");
   QLabel *x2Label        = new QLabel("X");
   QLabel *y2Label        = new QLabel("Y");
   QLabel *z2Label        = new QLabel("Z");

   FluoSpinbox *x1SSpinbox     = new FluoSpinbox(0,512,false);
   FluoSpinbox *x1MSpinbox     = new FluoSpinbox(0,512,false);
   FluoSpinbox *y1GSpinbox     = new FluoSpinbox(0,512,false);
   FluoSpinbox *y1YSpinbox     = new FluoSpinbox(0,512,false);
   FluoSpinbox *z1PSpinbox     = new FluoSpinbox(0,512,false);
   FluoSpinbox *z1TSpinbox     = new FluoSpinbox(0,512,false);
   FluoSpinbox *setClipSpinbox = new FluoSpinbox(1,99,false);
   FluoSpinbox *slabSpinbox    = new FluoSpinbox(1,99,false);
   FluoSpinbox *widthSpinbox   = new FluoSpinbox(1,99,false);

   FluoColoredLine *salmonHLine = new FluoColoredLine(QFrame::HLine, "Salmon");
   FluoColoredLine *salmonVLine = new FluoColoredLine(QFrame::VLine, "Salmon");
   FluoColoredLine *magentaHLine = new FluoColoredLine(QFrame::HLine, "Magenta");
   FluoColoredLine *greenHLine = new FluoColoredLine(QFrame::HLine, "Green");
   FluoColoredLine *greenVLine = new FluoColoredLine(QFrame::VLine, "Green");
   FluoColoredLine *yellowHLine = new FluoColoredLine(QFrame::HLine, "Yellow");
   FluoColoredLine *purpleHLine = new FluoColoredLine(QFrame::HLine, "Purple");
   FluoColoredLine *purpleVLine = new FluoColoredLine(QFrame::VLine, "Purple");
   FluoColoredLine *tealHLine = new FluoColoredLine(QFrame::HLine, "Teal");

   FluoSlider *x1SSlider = new FluoSlider(Qt::Vertical,0,512);
   FluoSlider *x1MSlider = new FluoSlider(Qt::Vertical,0,512);
   FluoSlider *y1GSlider = new FluoSlider(Qt::Vertical,0,512);
   FluoSlider *y1YSlider = new FluoSlider(Qt::Vertical,0,512);
   FluoSlider *z1PSlider = new FluoSlider(Qt::Vertical,0,512);
   FluoSlider *z1TSlider = new FluoSlider(Qt::Vertical,0,512);
   FluoSlider *x2Slider  = new FluoSlider(Qt::Vertical,-180,180);
   FluoSlider *y2Slider  = new FluoSlider(Qt::Vertical,-180,180);
   FluoSlider *z2Slider  = new FluoSlider(Qt::Vertical,-180,180);

   QPushButton *yzButton = new QPushButton("YZ");
   QPushButton *xzButton = new QPushButton("XZ");
   QPushButton *xyButton = new QPushButton("XY");
   QPushButton *resetClipsButton = new QPushButton(QIcon(":/clipping-reset.svg")," Reset Clips");
   QPushButton *alignToViewButton = new QPushButton("Align to View");
   QPushButton *resetTo0Button = new QPushButton(QIcon(":/clipping-reset.svg")," Reset to 0");

   FluoSpinboxDouble *x2Spinbox = new FluoSpinboxDouble(-180,180,false);
   FluoSpinboxDouble *y2Spinbox = new FluoSpinboxDouble(-180,180,false);
   FluoSpinboxDouble *z2Spinbox = new FluoSpinboxDouble(-180,180,false);

   void row0()
   {
     this->addWidget(linkToolButton,0,3);
     this->addWidget(alwaysToolButton,0,4);
     this->addWidget(toggleToolButton,0,5);
   }
};

#endif
