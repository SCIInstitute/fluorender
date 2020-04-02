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
#include <CustomWidgets/controller.hpp>

#include <cmath>
#include <vector>
#include <tuple>
#include <functional>

class ClippingLayout : public QGridLayout
{
  Q_OBJECT

  public:
    ClippingLayout();

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

    void row0();
    void constructLayout();

    const std::vector<std::function<void()>> rowFuncs = {
      std::bind(&ClippingLayout::row0,this),
      std::bind(&ClippingLayout::addSingle<FluoLine>,this,1,lineSep1),
      std::bind(&ClippingLayout::addThreeToRowShift<QLabel>,this,2,x1Label,y1Label,z1Label),
      std::bind(&ClippingLayout::addThreeToRow<FluoSpinbox>,this,3,x1SSpinbox,y1GSpinbox,z1PSpinbox),
      std::bind(&ClippingLayout::addThreeToRow<FluoColoredLine>,this,4,salmonHLine,greenHLine,purpleHLine),
      std::bind(&ClippingLayout::addRow<FluoSlider,FluoColoredLine,FluoSlider,FluoSlider,FluoColoredLine,FluoSlider,FluoSlider,FluoColoredLine,FluoSlider>,
                 this,5,x1SSlider,salmonVLine,x1MSlider,y1GSlider,greenVLine,y1YSlider,z1PSlider,purpleVLine,z1TSlider),
      std::bind(&ClippingLayout::addThreeToRow<FluoColoredLine>,this,6,magentaHLine,yellowHLine,tealHLine),
      std::bind(&ClippingLayout::addThreeToRow<FluoSpinbox>,this,7,x1MSpinbox,y1YSpinbox,z1TSpinbox),
      std::bind(&ClippingLayout::addThreeToRowShift<FluoToolButton>,this,8,xChainToolbutton,yChainToolbutton,zChainToolbutton),
      std::bind(&ClippingLayout::addSingle<FluoLine>,this,9,lineSep2),
      std::bind(&ClippingLayout::addThreeToRowShift<QLabel>,this,10,setClipLabel,slabLabel,widthLabel),
      std::bind(&ClippingLayout::addThreeToRow<QPushButton>,this,11,yzButton,xzButton,xyButton),
      std::bind(&ClippingLayout::addThreeToRow<FluoSpinbox>,this,12,setClipSpinbox,slabSpinbox,widthSpinbox),
      std::bind(&ClippingLayout::addSingle<QPushButton>,this,13,resetClipsButton),
      std::bind(&ClippingLayout::addSingle<FluoLine>,this,14,lineSep3),
      std::bind(&ClippingLayout::addSingle<QLabel>,this,15,rotationsLabel),
      std::bind(&ClippingLayout::addSingle<QPushButton>,this,16,alignToViewButton),
      std::bind(&ClippingLayout::addSingle<QPushButton>,this,17,resetTo0Button),
      std::bind(&ClippingLayout::addThreeToRowShift<QLabel>,this,18,x2Label,y2Label,z2Label),
      std::bind(&ClippingLayout::addThreeToRow<FluoSpinboxDouble>,this,19,z2Spinbox,y2Spinbox,x2Spinbox),
      std::bind(&ClippingLayout::addThreeToRowShift<FluoSlider>,this,20,z2Slider,y2Slider,x2Slider)
    };

    Controller<FluoSpinbox,FluoSlider> *x1SalmonController =
      new Controller<FluoSpinbox,FluoSlider>(*x1SSpinbox, *x1SSlider);
    Controller<FluoSpinbox,FluoSlider> *y1GreenController = 
      new Controller<FluoSpinbox,FluoSlider>(*y1GSpinbox,*y1GSlider);
    Controller<FluoSpinbox,FluoSlider> *z1PurpleController = 
      new Controller<FluoSpinbox,FluoSlider>(*z1PSpinbox,*z1PSlider);
    Controller<FluoSpinbox,FluoSlider> *x1MagentaController =
      new Controller<FluoSpinbox,FluoSlider>(*x1MSpinbox,*x1MSlider);
    Controller<FluoSpinbox,FluoSlider> *y1YellowController = 
      new Controller<FluoSpinbox,FluoSlider>(*y1YSpinbox,*y1YSlider);
    Controller<FluoSpinbox,FluoSlider> *z1TealController =
      new Controller<FluoSpinbox,FluoSlider>(*z1TSpinbox,*z1TSlider);
    Controller<FluoSpinboxDouble,FluoSlider> *x2RotationController =
      new Controller<FluoSpinboxDouble,FluoSlider>(*x2Spinbox,*x2Slider);
    Controller<FluoSpinboxDouble,FluoSlider> *y2RotationController = 
      new Controller<FluoSpinboxDouble,FluoSlider>(*y2Spinbox,*y2Slider);
    Controller<FluoSpinboxDouble,FluoSlider> *z2RotationController =
      new Controller<FluoSpinboxDouble,FluoSlider>(*z2Spinbox,*z2Slider);
};

#endif
