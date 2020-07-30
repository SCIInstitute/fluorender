#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include <QTabWidget>

#include "volumePropertiesOptions.hpp"
#include "volumePropertiesMisc.hpp"
#include "meshPropertiesOptions.hpp"
#include "meshPropertiesMaterials.hpp"

#include <VolumeData/VolumeData.hpp>
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
    void onVolumeLoaded(int renderviewID, fluo::VolumeData *vd);
    void onMeshLoaded(int renderviewID);

    // Volume Functions
    void onGammaReceived(int value) { setPropGammaValue(value); }
    void onGammaReceived(double value) { setPropGammaValue(value); }
    void onExtBoundReceived(int value) { setPropExtBoundValue(value); }
    void onExtBoundReceived(double value) { setPropExtBoundValue(value); }
    void onSaturReceived(int value) { setPropSatValue(value); }
    void onLowThrReceived(int value) { setPropLowThreshValue(value); }
    void onHighThReceived(int value) { setPropHighThreshValue(value); }
    void onLuminReceived(int value) { setPropLuminValue(value); }
    void onShadReceived(int value) { setPropShadowValue(value); }
    void onShadReceived(double value) { setPropShadowValue(value); }
    void onShadBoolReceived(bool status) { setPropShadowEnabled(status); }
    void onAlphaReceived(int value) { setPropAlphaValue(value); }
    void onAlphaBoolReceived(bool status) { setPropAlphaEnabled(status); }
    void onSampleReceived(int value) { setPropSampleValue(value); }
    void onSampleReceived(double value) { setPropSampleValue(value); }
    void onLowShadReceived(int value) { setPropLowShaderValue(value); }
    void onLowShadReceived(double value) { setPropLowShaderValue(value); }
    void onHighShadReceived(int value) { setPropHighShaderValue(value); }
    void onHighShadReceived(double value) { setPropHighShaderValue(value); }
    void onShaderBoolReceived(bool status) { setPropShaderEnabled(status); }
    void onLowColReceived(int value) { setPropLowColorModeValue(value); }
    void onHighColReceived(int value) { setPropHighColorModeValue(value); }
    void onCMBoolReceived(bool status) { setPropCMEnabled(status); }

    // Mesh Functions
    void onTransparencyReceived(int value) { setPropTransValue(value); }
    void onTransparencyReceived(double value) { setPropTransValue(value); }
    void onShadMReceived(int value) { setPropShadowValueM(value); }
    void onShadMReceived(double value) { setPropShadowValueM(value); }
    void onLightingReceived(int value) { setPropLightingValue(value); }
    void onLightingReceived(double value) { setPropLightingValue(value); }
    void onSizeLimitReceived(int value) { setPropSizeLimitValue(value); }
    void onShininessReceived(int value) { setPropShininessValue(value); }

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

    // Mesh Functions
    template<typename T>
    void setPropTransValue(T newVal)
    {
      MeshPropertiesOptions* temp = getMeshPropertiesOptions();

      temp->setTransparancyValue(newVal);
    }

    template<typename T>
    void setPropShadowValueM(T newVal)
    {
      MeshPropertiesOptions* temp = getMeshPropertiesOptions();
      temp->setShadowValue(newVal);
    }

    template<typename T>
    void setPropLightingValue(T newVal)
    {
      MeshPropertiesOptions* temp = getMeshPropertiesOptions();
      temp->setLightingValue(newVal);
    }

    void setPropSizeLimitValue(int newVal);
    void setPropShininessValue(int newVal);


  private:

    typedef void(PropertiesPanel::*propIntFunc)(int);
    typedef void(PropertiesPanel::*propDblFunc)(double);
    typedef void(PropertiesPanel::*propBoolFunc)(bool);
    typedef void(VolumePropertiesOptions::*volIntFunc)(int);
    typedef void(VolumePropertiesOptions::*volDblFunc)(double);
    typedef void(VolumePropertiesOptions::*volBoolFunc)(bool);
    typedef void(MeshPropertiesOptions::*meshIntFunc)(int);
    typedef void(MeshPropertiesOptions::*meshDblFunc)(double);


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

    MeshPropertiesOptions* getMeshPropertiesOptions();
    MeshPropertiesMaterials* getMeshPropertiesMaterials();

    void makeVolumeConnections(VolumePropertiesOptions* layout);
    void createVolumeIntConnections(VolumePropertiesOptions* layout);
    void createVolumeDblConnections(VolumePropertiesOptions* layout);

    void makeMeshConnections(MeshPropertiesOptions* opLayout, MeshPropertiesMaterials* maLayout);
    void createMeshIntConnections(MeshPropertiesOptions* opLayout, MeshPropertiesMaterials* maLayout);
    void createMeshDblConnections(MeshPropertiesOptions* opLayout);

    const std::vector<std::tuple<volDblFunc, propDblFunc>> volumeDblConnections = {
      std::make_tuple(static_cast<volDblFunc>(&VolumePropertiesOptions::sendGammaValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onGammaReceived)),
      std::make_tuple(static_cast<volDblFunc>(&VolumePropertiesOptions::sendExtBoundValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onExtBoundReceived)),
      std::make_tuple(static_cast<volDblFunc>(&VolumePropertiesOptions::sendShadowValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onShadReceived)),
      std::make_tuple(static_cast<volDblFunc>(&VolumePropertiesOptions::sendSampleValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onSampleReceived)),
      std::make_tuple(static_cast<volDblFunc>(&VolumePropertiesOptions::sendLowShadeValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onLowShadReceived)),
      std::make_tuple(static_cast<volDblFunc>(&VolumePropertiesOptions::sendHighShadeValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onHighShadReceived))
    };

    const std::vector<std::tuple<volIntFunc, propIntFunc>> volumeIntConnections = {
      std::make_tuple(&VolumePropertiesOptions::sendSaturationValue,&PropertiesPanel::onSaturReceived),
      std::make_tuple(&VolumePropertiesOptions::sendLowThreshValue,&PropertiesPanel::onLowThrReceived),
      std::make_tuple(&VolumePropertiesOptions::sendHighThreshValue,&PropertiesPanel::onHighThReceived),
      std::make_tuple(&VolumePropertiesOptions::sendLuminanceValue,&PropertiesPanel::onLuminReceived),
      std::make_tuple(&VolumePropertiesOptions::sendAlphaValue,&PropertiesPanel::onAlphaReceived),
      std::make_tuple(&VolumePropertiesOptions::sendColorMapLowValue,&PropertiesPanel::onLowColReceived),
      std::make_tuple(&VolumePropertiesOptions::sendColorMapHighValue,&PropertiesPanel::onHighColReceived),
      
      std::make_tuple(static_cast<volIntFunc>(&VolumePropertiesOptions::sendGammaValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onGammaReceived)),
      std::make_tuple(static_cast<volIntFunc>(&VolumePropertiesOptions::sendExtBoundValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onExtBoundReceived)),
      std::make_tuple(static_cast<volIntFunc>(&VolumePropertiesOptions::sendShadowValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onShadReceived)),
      std::make_tuple(static_cast<volIntFunc>(&VolumePropertiesOptions::sendSampleValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onSampleReceived)),
      std::make_tuple(static_cast<volIntFunc>(&VolumePropertiesOptions::sendLowShadeValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onLowShadReceived)),
      std::make_tuple(static_cast<volIntFunc>(&VolumePropertiesOptions::sendHighShadeValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onHighShadReceived))
    };

    const std::vector<std::tuple<meshIntFunc, propIntFunc>> meshIntConnections = {
      std::make_tuple(static_cast<meshIntFunc>(&MeshPropertiesOptions::sendTransparencyValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onTransparencyReceived)),
      std::make_tuple(static_cast<meshIntFunc>(&MeshPropertiesOptions::sendShadowValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onShadMReceived)),
      std::make_tuple(static_cast<meshIntFunc>(&MeshPropertiesOptions::sendLightingValue),
                      static_cast<propIntFunc>(&PropertiesPanel::onLightingReceived))
    };
    
    const std::vector<std::tuple<meshDblFunc, propDblFunc>> meshDblConnections = {
      std::make_tuple(static_cast<meshDblFunc>(&MeshPropertiesOptions::sendTransparencyValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onTransparencyReceived)),
      std::make_tuple(static_cast<meshDblFunc>(&MeshPropertiesOptions::sendShadowValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onShadMReceived)),
      std::make_tuple(static_cast<meshDblFunc>(&MeshPropertiesOptions::sendLightingValue),
                      static_cast<propDblFunc>(&PropertiesPanel::onLightingReceived))
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
