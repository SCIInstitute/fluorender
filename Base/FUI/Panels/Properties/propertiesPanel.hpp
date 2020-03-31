#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include <QTabWidget>

#include "volumePropertiesOptions.hpp"
#include "volumePropertiesMisc.hpp"
#include "meshPropertiesOptions.hpp"
#include "meshPropertiesMaterials.hpp"

#include <Panels/Properties/Agent/volumePropAgent.hpp>


/*
 *
 * TODO: See if there is a way to inherit from a QMainWindow to use QDockWidgets
 *       instead. These tab widgets are not undockable so there is no way to
 *       take them off the dock.
 *
*/


class PropertiesPanel : public QWidget
{
  Q_OBJECT

  public slots:
    void onVolumeLoaded(int renderviewID);
    void onMeshLoaded(int renderviewID);

    void onGammaReceived(std::any value) { setPropGammaValue(value); }
    void onExtBoundReceived(std::any value) { setPropExtBoundValue(value); }
    void onSaturReceived(int value) { setPropSatValue(value); }
    void onLowThrReceived(int value) { setPropLowThreshValue(value); }
    void onHighThReceived(int value) { setPropHighThreshValue(value); }
    void onLuminReceived(int value) { setPropLuminValue(value); }
    void onShadReceived(std::any value) { setPropShadowValue(value); }
    void onShadBoolReceived(bool status) { setPropShadowEnabled(status); }
    void onAlphaReceived(int value) { setPropAlphaValue(value); }
    void onAlphaBoolReceived(bool status) { setPropAlphaEnabled(status); }
    void onSampleReceived(std::any value) { setPropSampleValue(value); }
    void onLowShadReceived(std::any value) { setPropLowShaderValue(value); }
    void onHighShadReceived(std::any value) { setPropHighShaderValue(value); }
    void onShaderBoolReceived(bool status) { setPropShaderEnabled(status); }
    void onLowColReceived(int value) { setPropLowColorModeValue(value); }
    void onHighColReceived(int value) { setPropHighColorModeValue(value); }
    void onCMBoolReceived(bool status) { setPropCMEnabled(status); }

    /*
    void onExtBoundReceived(std::any value) { setPropExtBoundValue(value); }

    void onSatReceived(int value) { setPropSatValue(value); }
    void onLowThreshreceived(int value) { setPropLowThreshValue(value); }
    */
  public:
    PropertiesPanel();

    void setPropOptionsMaxVal(double newVal);
    double getPropOptionsMaxVal() const;

    template<typename T>
    void setPropGammaValue(T newVal)
    {
      VolumePropertiesOptions* temp = getPropertiesOptions();

      temp->setGammaValue(newVal);
    }

    template<typename T>
    void setPropExtBoundValue(T newVal)
    {
      VolumePropertiesOptions* temp = getPropertiesOptions();

      temp->setExtBoundValue(newVal);
    }

    void setPropSatValue(int newVal);

    void setPropLowThreshValue(int newVal);

    void setPropHighThreshValue(int newVal);

    void setPropLuminValue(int newVal);

    template<typename T>
    void setPropShadowValue(T newVal)
    {
      VolumePropertiesOptions* temp = getPropertiesOptions();

      temp->setShadowValue(newVal);
    }

    void setPropShadowEnabled(bool status);

    void setPropAlphaValue(int newVal);
    void setPropAlphaEnabled(bool status);

    template<typename T>
    void setPropSampleValue(T newVal)
    {
      VolumePropertiesOptions* temp = getPropertiesOptions();

      temp->setSampleValue(newVal);
    }

    template<typename T>
    void setPropLowShaderValue(T newVal)
    {
      VolumePropertiesOptions* temp = getPropertiesOptions();

      temp->setLowShaderVal(newVal);
    }

    template<typename T>
    void setPropHighShaderValue(T newVal)
    {
      VolumePropertiesOptions* temp = getPropertiesOptions();

      temp->setHighShaderVal(newVal);
    }

    void setPropShaderEnabled(bool status);

    void setPropLowColorModeValue(int newVal);

    void setPropHighColorModeValue(int newVal);

    void setPropCMEnabled(bool status);


  private:

    typedef void(PropertiesPanel::*propAnyFunc)(std::any);
    typedef void(PropertiesPanel::*propIntFunc)(int);
    typedef void(PropertiesPanel::*propBoolFunc)(bool);
    typedef void(VolumePropertiesOptions::*volAnyFunc)(std::any);
    typedef void(VolumePropertiesOptions::*volIntFunc)(int);
    typedef void(VolumePropertiesOptions::*volBoolFunc)(bool);

    template<typename Property>
    QFrame *genLeftFrame(Property* left)
    {
      QFrame *leftFrame = new QFrame();
      leftFrame->setLayout(left);

      return leftFrame;
    }

    template<typename Property>
    QFrame *genRightFrame(Property* right)
    {
      QFrame *rightFrame = new QFrame();
      rightFrame->setLayout(right);

      return rightFrame;
    }

    QWidget *genMainWidget(QFrame *left, QFrame *right);

    QTabWidget *tabWidget = new QTabWidget();
    QGridLayout *myLayout = new QGridLayout();

    VolumePropertiesOptions* getPropertiesOptions();
    VolumePropertiesMisc* getPropertiesMisc();

    void makeVolumeConnections(VolumePropertiesOptions* layout);
    void createVolumeAnyConnections(VolumePropertiesOptions* layout);
    void createVolumeIntConnections(VolumePropertiesOptions* layout);

    const std::vector<std::tuple<volAnyFunc, propAnyFunc>> volumeAnyConnections = {
      std::make_tuple(&VolumePropertiesOptions::sendGammaValue,&PropertiesPanel::onGammaReceived),
      std::make_tuple(&VolumePropertiesOptions::sendExtBoundValue,&PropertiesPanel::onExtBoundReceived),
      std::make_tuple(&VolumePropertiesOptions::sendShadowValue,&PropertiesPanel::onShadReceived),
      std::make_tuple(&VolumePropertiesOptions::sendSampleValue,&PropertiesPanel::onSampleReceived),
      std::make_tuple(&VolumePropertiesOptions::sendLowShadeValue,&PropertiesPanel::onLowShadReceived),
      std::make_tuple(&VolumePropertiesOptions::sendHighShadeValue,&PropertiesPanel::onHighShadReceived)
    };

    const std::vector<std::tuple<volIntFunc, propIntFunc>> volumeIntConnections = {
      std::make_tuple(&VolumePropertiesOptions::sendSaturationValue,&PropertiesPanel::onSaturReceived),
      std::make_tuple(&VolumePropertiesOptions::sendLowThreshValue,&PropertiesPanel::onLowThrReceived),
      std::make_tuple(&VolumePropertiesOptions::sendHighThreshValue,&PropertiesPanel::onHighThReceived),
      std::make_tuple(&VolumePropertiesOptions::sendLuminanceValue,&PropertiesPanel::onLuminReceived),
      std::make_tuple(&VolumePropertiesOptions::sendAlphaValue,&PropertiesPanel::onAlphaReceived),
      std::make_tuple(&VolumePropertiesOptions::sendColorMapLowValue,&PropertiesPanel::onLowColReceived),
      std::make_tuple(&VolumePropertiesOptions::sendColorMapHighValue,&PropertiesPanel::onHighColReceived)
    };

/*
    const std::vector<std::tuple<volBoolFunc, propBoolFunc>> volumeBoolConnections = {
      std::make_tuple(&VolumePropertiesOptions::send)
    };
    void onShadBoolReceived(bool status) { setPropShadowEnabled(status); }
    void onAlphaBoolReceived(bool status) { setPropAlphaEnabled(status); }
    void onShaderBoolReceived(bool status) { setPropShaderEnabled(status); }
*/

    VolumePropAgent* m_agent;
    friend class VolumePropAgent;
};

#endif
