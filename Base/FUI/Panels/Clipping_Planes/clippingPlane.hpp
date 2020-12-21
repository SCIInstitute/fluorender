#ifndef CLIPPING_PLANE_HPP
#define CLIPPING_PLANE_HPP

#include <QWidget>
#include <QGroupBox>

#include "clippingLayout.hpp"

#include <Panels/Clipping_Planes/Agent/clipPlaneAgent.hpp>

class ClippingPlane : public QWidget
{
  Q_OBJECT

  public slots:
    void onSalmonValueReceived(int value) { setClipSalmonValue(value); }
    void onMagentaValueReceived(int value) { setClipMagentaValue(value); }
    void onGreenValueReceived(int value) { setClipGreenValue(value); }
    void onYellowValueReceived(int value) { setClipYellowValue(value); }
    void onPurpleValueReceived(int value) { setClipPurpleValue(value); }
    void onTealValueReceived(int value) { setClipTealValue(value); }
    void onClipValueReceived(int value) { setClipClipValue(value); }
    void onSlabValueReceived(int value) { setClipSlabValue(value); }
    void onWidthValueReceived(int value) { setClipWidthValue(value); }

    void onRotXValueReceived(int value) { setClipXRotValue(value); }
    //void onRotXValueReceived(double value) { setClipXRotValue(value); }
    void onRotYValueReceived(int value) { setClipYRotValue(value); }
    //void onRotYValueReceived(double value) { setClipYRotValue(value); }
    void onRotZValueReceived(int value) { setClipZRotValue(value); }
    //void onRotZValueReceived(double value) { setClipZRotValue(value); }
    
    void onLockXStatusReceived(bool status) { setXPlaneLockStatus(status); }
    void onLockYStatusReceived(bool status) { setYPlaneLockStatus(status); }
    void onLockZStatusReceived(bool status) { setZPlaneLockStatus(status); }

  public: 
    ClippingPlane();

    void setClipSalmonValue(int value);
    void setClipMagentaValue(int value);
    void setClipGreenValue(int value);
    void setClipYellowValue(int value);
    void setClipPurpleValue(int value);
    void setClipTealValue(int value);
    void setClipClipValue(int value);
    void setClipSlabValue(int value);
    void setClipWidthValue(int value);

    //template<typename T>
    void setClipXRotValue(int value)
    {
      clippingLayout->setRotXValue(value);
    }

    //template<typename T>
    void setClipYRotValue(int value)
    {
      clippingLayout->setRotYValue(value);
    }

    //template<typename T>
    void setClipZRotValue(int value)
    {
      clippingLayout->setRotZValue(value);
    }

    void setXPlaneLockStatus(bool status);
    void setYPlaneLockStatus(bool status);
    void setZPlaneLockStatus(bool status);
    void setVolumeData(fluo::VolumeData* vd);

    const int getClipSalmonMaxVal();
    const int getClipGreenMaxVal();
    const int getClipPurpleMaxVal();

  private:

    typedef void(ClippingPlane::*clipIntFunc)(int);
    typedef void(ClippingPlane::*clipDblFunc)(double);
    typedef void(ClippingPlane::*clipBoolFunc)(bool);
    typedef void(ClippingLayout::*layoutIntFunc)(int);
    typedef void(ClippingLayout::*layoutDblFunc)(double);
    typedef void(ClippingLayout::*layoutBoolFunc)(bool);

    QGroupBox *outputFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    ClippingLayout *clippingLayout = new ClippingLayout();

    void createLayout();
    void makeIntConnections();
    //void makeDblConnections();
    void makeBoolConnections();

/*
    const std::vector<std::tuple<ClippingLayout*, layoutDblFunc, clipDblFunc>> dblConnections = {
      std::make_tuple(clippingLayout,static_cast<layoutDblFunc>(&ClippingLayout::sendRotXValue),
                      static_cast<clipDblFunc>(&ClippingPlane::onRotXValueReceived)),
      std::make_tuple(clippingLayout,static_cast<layoutDblFunc>(&ClippingLayout::sendRotYValue),
                      static_cast<clipDblFunc>(&ClippingPlane::onRotYValueReceived)),
      std::make_tuple(clippingLayout,static_cast<layoutDblFunc>(&ClippingLayout::sendRotZValue),
                      static_cast<clipDblFunc>(&ClippingPlane::onRotZValueReceived))
    };
*/    
    const std::vector<std::tuple<ClippingLayout*, layoutBoolFunc, clipBoolFunc>> boolConnections = {
      std::make_tuple(clippingLayout,&ClippingLayout::sendXPlaneBoolean,&ClippingPlane::onLockXStatusReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendYPlaneBoolean,&ClippingPlane::onLockYStatusReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendZPlaneBoolean,&ClippingPlane::onLockZStatusReceived)
    };

    const std::vector<std::tuple<ClippingLayout*, layoutIntFunc, clipIntFunc>> intConnections = {
      std::make_tuple(clippingLayout,&ClippingLayout::sendSalmonValue,&ClippingPlane::onSalmonValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendMagentaValue,&ClippingPlane::onMagentaValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendGreenValue,&ClippingPlane::onGreenValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendYellowValue,&ClippingPlane::onYellowValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendPurpleValue,&ClippingPlane::onPurpleValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendTealValue,&ClippingPlane::onTealValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendClipValue,&ClippingPlane::onClipValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendSlabValue,&ClippingPlane::onSlabValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendWidthValue,&ClippingPlane::onWidthValueReceived),
      std::make_tuple(clippingLayout,static_cast<layoutIntFunc>(&ClippingLayout::sendRotXValue),
                      static_cast<clipIntFunc>(&ClippingPlane::onRotXValueReceived)),
      std::make_tuple(clippingLayout,static_cast<layoutIntFunc>(&ClippingLayout::sendRotYValue),
                      static_cast<clipIntFunc>(&ClippingPlane::onRotYValueReceived)),
      std::make_tuple(clippingLayout,static_cast<layoutIntFunc>(&ClippingLayout::sendRotZValue),
                      static_cast<clipIntFunc>(&ClippingPlane::onRotZValueReceived))
    };
    
    ClipPlaneAgent* m_agent;
    friend class ClipPlaneAgent;
};



#endif
