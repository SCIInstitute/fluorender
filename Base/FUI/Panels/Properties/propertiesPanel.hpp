#ifndef PROPERTIES_PANEL_HPP
#define PROPERTIES_PANEL_HPP

#include <QTabWidget>

#include "volumePropertiesOptions.hpp"
#include "volumePropertiesMisc.hpp"
#include "meshPropertiesOptions.hpp"
#include "meshPropertiesMaterials.hpp"

#include <Panels/Properties/Agent/volumePropAgent.hpp>

#include <iostream>

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

    VolumePropAgent* m_agent;
    friend class VolumePropAgent;

};

#endif
