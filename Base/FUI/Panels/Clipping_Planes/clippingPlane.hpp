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

    void onRotXValueReceived(std::any value) { setClipXRotValue(value); }
    void onRotYValueReceived(std::any value) { setClipYRotValue(value); }
    void onRotZValueReceived(std::any value) { setClipZRotValue(value); }

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

    template<typename T>
    void setClipXRotValue(T value)
    {
      ClippingLayout *temp = getClipLayout();
      temp->setRotXValue(value);
    }

    template<typename T>
    void setClipYRotValue(T value)
    {
      ClippingLayout *temp = getClipLayout();
      temp->setRotYValue(value);
    }

    template<typename T>
    void setClipZRotValue(T value)
    {
      ClippingLayout *temp = getClipLayout();
      temp->setRotZValue(value);
    }

  private:

    typedef void(ClippingPlane::*clipAnyFunc)(std::any);
    typedef void(ClippingPlane::*clipIntFunc)(int);
    typedef void(ClippingLayout::*layoutAnyFunc)(std::any);
    typedef void(ClippingLayout::*layoutIntFunc)(int);

    QGroupBox *outputFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    ClippingLayout *clippingLayout = new ClippingLayout();

    ClippingLayout *getClipLayout();

    void makeStdAnyConnections();
    void makeIntConnections();

    const std::vector<std::tuple<ClippingLayout*, layoutAnyFunc, clipAnyFunc>> stdAnyConnections = {
      std::make_tuple(clippingLayout,&ClippingLayout::sendRotXValue,&ClippingPlane::onRotXValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendRotYValue,&ClippingPlane::onRotYValueReceived),
      std::make_tuple(clippingLayout,&ClippingLayout::sendRotZValue,&ClippingPlane::onRotZValueReceived)
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
      std::make_tuple(clippingLayout,&ClippingLayout::sendWidthValue,&ClippingPlane::onWidthValueReceived)
    };
    
    ClipPlaneAgent* m_agent;
    friend class ClipPlaneAgent;
};



#endif
