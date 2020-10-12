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
#include <CustomWidgets/agentWrapper.hpp>

#include <cmath>
#include <vector>
#include <tuple>
#include <functional>

#include <Panels/Clipping_Planes/Agent/clipPlaneAgent.hpp>

class ClippingLayout : public QGridLayout
{
  Q_OBJECT

  signals:
    void sendSalmonValue(int value);
    void sendMagentaValue(int value);
    void sendGreenValue(int value);
    void sendYellowValue(int value);
    void sendPurpleValue(int value);
    void sendTealValue(int value);

    void sendClipValue(int value);
    void sendSlabValue(int value);
    void sendWidthValue(int value);

    void sendRotXValue(int value);
    void sendRotXValue(double value);
    void sendRotYValue(int value);
    void sendRotYValue(double value);
    void sendRotZValue(int value);
    void sendRotZValue(double value);

    void sendXPlaneBoolean(bool status);
    void sendYPlaneBoolean(bool status);
    void sendZPlaneBoolean(bool status);

  public slots:
    void onSalmonSliderChanged() { sendSalmonValue(x1SSlider->value()); }
    void onMagentaSliderChanged() { sendMagentaValue(x1MSlider->value()); }
    void onGreenSliderChanged() { sendGreenValue(y1GSlider->value()); }
    void onYellowSliderChanged() { sendYellowValue(y1YSlider->value()); }
    void onPurpleSliderChanged() { sendPurpleValue(z1PSlider->value()); }
    void onTealSliderChanged() { sendTealValue(z1TSlider->value()); }

    void onSalmonSpinboxChanged() { sendSalmonValue(x1SSpinbox->value()); }
    void onMagentaSpinboxChanged() { sendMagentaValue(x1MSpinbox->value()); }
    void onGreenSpinboxChanged() { sendGreenValue(y1GSpinbox->value()); }
    void onYellowSpinboxChanged() { sendYellowValue(y1YSpinbox->value()); }
    void onPurpleSpinboxChanged() { sendPurpleValue(z1PSpinbox->value()); }
    void onTealSpinboxChanged() { sendTealValue(z1TSpinbox->value()); }

    void onClipSpinboxChanged() { sendClipValue(setClipSpinbox->value()); }
    void onSlabSpinboxChanged() { sendSlabValue(slabSpinbox->value()); }
    void onWidthSpinboxChanged() { sendWidthValue(widthSpinbox->value()); }

    void onRotXSliderChanged() { sendRotXValue(x2Slider->value()); }
    void onRotYSliderChanged() { sendRotYValue(y2Slider->value()); }
    void onRotZSliderChanged() { sendRotZValue(z2Slider->value()); }

    void onRotXSpinboxChanged() { sendRotXValue(x2Spinbox->value()); }
    void onRotYSpinboxChanged() { sendRotYValue(y2Spinbox->value()); }
    void onRotZSpinboxChanged() { sendRotZValue(z2Spinbox->value()); }

    void onXPlaneLockToggled() { sendXPlaneBoolean(xChainToolbutton->isChecked()); }
    void onYPlaneLockToggled() { sendYPlaneBoolean(yChainToolbutton->isChecked()); }
    void onZPlaneLockToggled() { sendZPlaneBoolean(zChainToolbutton->isChecked()); }

  public:
    ClippingLayout();

    void setSalmonValue(int newVal); //{ x1SalmonController->setValues(newVal); }
    void setMagentaValue(int newVal); //{ x1MagentaController->setValues(newVal); }
    void setGreenValue(int newVal); //{ y1GreenController->setValues(newVal); }
    void setYellowValue(int newVal); //{ y1YellowController->setValues(newVal); }
    void setPurpleValue(int newVal); //{ z1PurpleController->setValues(newVal); }
    void setTealValue(int newVal); //{ z1TealController->setValues(newVal); }

    void setClipValue(int newVal) { setClipSpinbox->setValue(newVal); }
    void setSlabValue(int newVal) { slabSpinbox->setValue(newVal); }
    void setWidthValue(int newVal) { widthSpinbox->setValue(newVal); }

    template<typename T>
    void setRotXValue(T newVal) { x2RotationController->setValues(newVal); }

    template<typename T>
    void setRotYValue(T newVal) { y2RotationController->setValues(newVal); }

    template<typename T>
    void setRotZValue(T newVal) { z2RotationController->setValues(newVal); }

    const int getSalmonSliderMax() { return x1SSlider->maximum(); }
    const int getGreenSliderMax() { return y1GSlider->maximum(); }
    const int getPurpleSliderMax() { return z1PSlider->maximum(); }

    void setAgent(ClipPlaneAgent *agent, fluo::VolumeData *vd);
    void disableLayout();
    void enableLayout();
    void build();

  private:

    ClipPlaneAgent *m_agent = nullptr;

    typedef void(ClippingLayout::*clipFunc)();
    typedef void(FluoSlider::*sliderFunc)(int);
    typedef void(FluoSpinbox::*spinFunc)();
    typedef void(FluoSpinboxDouble::*dSpinFunc)();

    const int MAXCOLSIZE = 8;
    const int XYMAX = 640;
    const int ZMAX = 88;
    const int HALFXY = XYMAX/2;
    const int HALFZ = ZMAX/2;

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

    FluoSpinbox *x1SSpinbox     = new FluoSpinbox(0,640,false);
    FluoSpinbox *x1MSpinbox     = new FluoSpinbox(0,640,false);
    FluoSpinbox *y1GSpinbox     = new FluoSpinbox(0,640,false);
    FluoSpinbox *y1YSpinbox     = new FluoSpinbox(0,640,false);
    FluoSpinbox *z1PSpinbox     = new FluoSpinbox(0,88,false);
    FluoSpinbox *z1TSpinbox     = new FluoSpinbox(0,88,false);
    FluoSpinbox *setClipSpinbox = new FluoSpinbox(0,640,false);
    FluoSpinbox *slabSpinbox    = new FluoSpinbox(0,640,false);
    FluoSpinbox *widthSpinbox   = new FluoSpinbox(0,88,false);

    FluoColoredLine *salmonHLine = new FluoColoredLine(QFrame::HLine, "Salmon");
    FluoColoredLine *salmonVLine = new FluoColoredLine(QFrame::VLine, "Salmon");
    FluoColoredLine *magentaHLine = new FluoColoredLine(QFrame::HLine, "Magenta");
    FluoColoredLine *greenHLine = new FluoColoredLine(QFrame::HLine, "Green");
    FluoColoredLine *greenVLine = new FluoColoredLine(QFrame::VLine, "Green");
    FluoColoredLine *yellowHLine = new FluoColoredLine(QFrame::HLine, "Yellow");
    FluoColoredLine *purpleHLine = new FluoColoredLine(QFrame::HLine, "Purple");
    FluoColoredLine *purpleVLine = new FluoColoredLine(QFrame::VLine, "Purple");
    FluoColoredLine *tealHLine = new FluoColoredLine(QFrame::HLine, "Teal");

    FluoSlider *x1SSlider = new FluoSlider(Qt::Vertical,0,640);
    FluoSlider *x1MSlider = new FluoSlider(Qt::Vertical,0,640);
    FluoSlider *y1GSlider = new FluoSlider(Qt::Vertical,0,640);
    FluoSlider *y1YSlider = new FluoSlider(Qt::Vertical,0,640);
    FluoSlider *z1PSlider = new FluoSlider(Qt::Vertical,0,88);
    FluoSlider *z1TSlider = new FluoSlider(Qt::Vertical,0,88);
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

    AgentWrapper<ClipPlaneAgent> *salmonWrapper;
    AgentWrapper<ClipPlaneAgent> *magentaWrapper;
    AgentWrapper<ClipPlaneAgent> *greenWrapper;
    AgentWrapper<ClipPlaneAgent> *yellowWrapper;
    AgentWrapper<ClipPlaneAgent> *purpleWrapper;
    AgentWrapper<ClipPlaneAgent> *tealWrapper;
    AgentWrapper<ClipPlaneAgent> *clipLinkXWrapper;
    AgentWrapper<ClipPlaneAgent> *clipLinkYWrapper;
    AgentWrapper<ClipPlaneAgent> *clipLinkZWrapper;
    AgentWrapper<ClipPlaneAgent> *clipRotXWrapper;
    AgentWrapper<ClipPlaneAgent> *clipRotYWrapper;
    AgentWrapper<ClipPlaneAgent> *clipRotZWrapper;

    void row0();
    void flipSliders(); // might make this default in the slider.
    void constructLayout();
    void buildSliderConnections();
    void buildSpinboxConnections();
    void buildSpinboxDConnections();
    void buildWrappers();
    void buildControllers();
    void disableButtons();
    void enableButtons();

    std::tuple<int,int> sliderGap(FluoSlider* s1, FluoSlider* s2, int size);
    

    const std::vector<std::tuple<FluoSlider*, sliderFunc, clipFunc>> sliderConnections = {
      std::make_tuple(x1SSlider, &FluoSlider::valueChanged, &ClippingLayout::onSalmonSliderChanged),
      std::make_tuple(x1MSlider, &FluoSlider::valueChanged, &ClippingLayout::onMagentaSliderChanged),
      std::make_tuple(y1GSlider, &FluoSlider::valueChanged, &ClippingLayout::onGreenSliderChanged),
      std::make_tuple(y1YSlider, &FluoSlider::valueChanged, &ClippingLayout::onYellowSliderChanged),
      std::make_tuple(z1PSlider, &FluoSlider::valueChanged, &ClippingLayout::onPurpleSliderChanged),
      std::make_tuple(z1TSlider, &FluoSlider::valueChanged, &ClippingLayout::onTealSliderChanged),
      std::make_tuple(x2Slider, &FluoSlider::valueChanged, &ClippingLayout::onRotXSliderChanged),
      std::make_tuple(y2Slider, &FluoSlider::valueChanged, &ClippingLayout::onRotYSliderChanged),
      std::make_tuple(z2Slider, &FluoSlider::valueChanged, &ClippingLayout::onRotZSliderChanged)
    };

    const std::vector<std::tuple<FluoSpinbox*, spinFunc, clipFunc>> spinConnections = {
      std::make_tuple(x1SSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onSalmonSpinboxChanged),
      std::make_tuple(x1MSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onMagentaSpinboxChanged),
      std::make_tuple(y1GSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onGreenSpinboxChanged),
      std::make_tuple(y1YSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onYellowSpinboxChanged),
      std::make_tuple(z1PSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onPurpleSpinboxChanged),
      std::make_tuple(z1TSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onTealSpinboxChanged),
      std::make_tuple(setClipSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onClipSpinboxChanged),
      std::make_tuple(slabSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onSlabSpinboxChanged),
      std::make_tuple(widthSpinbox, &FluoSpinbox::editingFinished, &ClippingLayout::onWidthSpinboxChanged),
    };

    const std::vector<std::tuple<FluoSpinboxDouble*, dSpinFunc, clipFunc>> dSpinConnections = {
      std::make_tuple(x2Spinbox, &FluoSpinboxDouble::editingFinished, &ClippingLayout::onRotXSpinboxChanged),
      std::make_tuple(y2Spinbox, &FluoSpinboxDouble::editingFinished, &ClippingLayout::onRotYSpinboxChanged),
      std::make_tuple(z2Spinbox, &FluoSpinboxDouble::editingFinished, &ClippingLayout::onRotZSpinboxChanged),
    };

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

    Controller<FluoSpinbox,FluoSlider,AgentWrapper<ClipPlaneAgent>> *x1SalmonController;
    Controller<FluoSpinbox,FluoSlider,AgentWrapper<ClipPlaneAgent>> *y1GreenController; 
    Controller<FluoSpinbox,FluoSlider,AgentWrapper<ClipPlaneAgent>> *z1PurpleController;  
    Controller<FluoSpinbox,FluoSlider,AgentWrapper<ClipPlaneAgent>> *x1MagentaController; 
    Controller<FluoSpinbox,FluoSlider,AgentWrapper<ClipPlaneAgent>> *y1YellowController; 
    Controller<FluoSpinbox,FluoSlider,AgentWrapper<ClipPlaneAgent>> *z1TealController; 
    Controller<FluoSpinboxDouble,FluoSlider,AgentWrapper<ClipPlaneAgent>> *x2RotationController; 
    Controller<FluoSpinboxDouble,FluoSlider,AgentWrapper<ClipPlaneAgent>> *y2RotationController;  
    Controller<FluoSpinboxDouble,FluoSlider,AgentWrapper<ClipPlaneAgent>> *z2RotationController; 
};

#endif
