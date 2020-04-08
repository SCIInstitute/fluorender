#ifndef MESH_PROPERTIES_OPTIONS
#define MESH_PROPERTIES_OPTIONS

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoToolButton.hpp>
#include <CustomWidgets/controller.hpp>

#include <vector>
#include <tuple>
#include <functional>

class MeshPropertiesOptions : public QGridLayout
{
  Q_OBJECT

  signals:

    void sendTransparencyValue(std::any value);
    void sendShadowValue(std::any value);
    void sendLightingValue(std::any value);
    void sendSizeLimitValue(int value);

  public slots:
    void onTransparencySliderChanged() { sendTransparencyValue(transparencySlider->value()); }
    void onTransparencySpinboxChanged() { sendTransparencyValue(transparencySpinbox->value()); }
    void onShadowSliderChanged() { sendShadowValue(shadowSlider->value()); }
    void onShadowSpinboxChanged() { sendShadowValue(shadowSpinbox->value()); }
    void onLightingSliderChanged() { sendLightingValue(lightingSlider->value()); }
    void onLightingSpinboxChanged() { sendLightingValue(lightingSpinbox->value()); }
    void onSizeLimitSliderChanged() { sendSizeLimitValue(sizeLimitSlider->value()); }
    void onSizeLimitSpinboxChanged() { sendSizeLimitValue(sizeLimitSpinbox->value()); }

  public:
    MeshPropertiesOptions();

    template<typename T>
    void setTransparancyValue(T newVal) { transparencyController->setValues(newVal); }

    template<typename T>
    void setShadowValue(T newVal) { shadowController->setValues(newVal); }

    template<typename T>
    void setLightingValue(T newVal) { lightingController->setValues(newVal); }
  
    void setSizeLimitValue(int newVal) { sizeLimitController->setValues(newVal); }

  private:

    typedef void(MeshPropertiesOptions::*meshFunc)();
    typedef void(FluoSlider::*sliderFunc)(int);
    typedef void(FluoSpinboxDouble::*dSpinFunc)();

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();

    void constructLayout();
    void createSliderConnections();
    void createSpinboxConnections();

    FluoSlider* transparencySlider = new FluoSlider(Qt::Horizontal,0,100);
    FluoSlider* shadowSlider       = new FluoSlider(Qt::Horizontal,0,100);
    FluoSlider* lightingSlider     = new FluoSlider(Qt::Horizontal,0,200);
    FluoSlider* sizeLimitSlider    = new FluoSlider(Qt::Horizontal,0,250);

    FluoSpinboxDouble* transparencySpinbox = new FluoSpinboxDouble(0.0,1.0,false);
    FluoSpinboxDouble* shadowSpinbox       = new FluoSpinboxDouble(0.0,1.0,false);
    FluoSpinboxDouble* lightingSpinbox     = new FluoSpinboxDouble(0.0,2.0,false);
    FluoSpinbox* sizeLimitSpinbox          = new FluoSpinbox(0,250,false);

    QLabel* transparencyLabel = new QLabel("Transparency: ");
    QLabel* shadowLabel       = new QLabel("Shadow: ");
    QLabel* lightingLabel     = new QLabel("Lighting Scaling: ");
    QLabel* sizeLimitLabel    = new QLabel("Size Limit: ");

    QCheckBox* shadowCheckbox    = new QCheckBox();
    QCheckBox* lightingCheckbox  = new QCheckBox();
    QCheckBox* sizeLimitCheckbox = new QCheckBox();

    const std::vector<std::function<void()>> rowFuncs = {
      std::bind(&MeshPropertiesOptions::addRow0,this),
      std::bind(&MeshPropertiesOptions::addRow1,this),
      std::bind(&MeshPropertiesOptions::addRow2,this),
      std::bind(&MeshPropertiesOptions::addRow3,this),
    };

    const std::vector<std::tuple<FluoSlider*, sliderFunc, meshFunc>> sliderConnections = {
      std::make_tuple(transparencySlider,&FluoSlider::valueChanged, &MeshPropertiesOptions::onTransparencySliderChanged),
      std::make_tuple(shadowSlider,&FluoSlider::valueChanged, &MeshPropertiesOptions::onShadowSliderChanged),
      std::make_tuple(lightingSlider,&FluoSlider::valueChanged, &MeshPropertiesOptions::onLightingSliderChanged),
      std::make_tuple(sizeLimitSlider,&FluoSlider::valueChanged, &MeshPropertiesOptions::onSizeLimitSliderChanged)
    };

    const std::vector<std::tuple<FluoSpinboxDouble*, dSpinFunc, meshFunc>> spinboxConnections = {
      std::make_tuple(transparencySpinbox,&FluoSpinboxDouble::editingFinished, &MeshPropertiesOptions::onTransparencySpinboxChanged),
      std::make_tuple(shadowSpinbox,&FluoSpinboxDouble::editingFinished, &MeshPropertiesOptions::onShadowSpinboxChanged),
      std::make_tuple(lightingSpinbox,&FluoSpinboxDouble::editingFinished, &MeshPropertiesOptions::onLightingSpinboxChanged),
    };

    Controller<FluoSlider,FluoSpinboxDouble> *transparencyController =
      new Controller<FluoSlider,FluoSpinboxDouble>(*transparencySlider,*transparencySpinbox);
    Controller<FluoSlider,FluoSpinboxDouble> *shadowController = 
      new Controller<FluoSlider,FluoSpinboxDouble>(*shadowSlider,*shadowSpinbox);
    Controller<FluoSlider,FluoSpinboxDouble> *lightingController = 
      new Controller<FluoSlider,FluoSpinboxDouble>(*lightingSlider,*lightingSpinbox);
    Controller<FluoSlider,FluoSpinbox> *sizeLimitController =
      new Controller<FluoSlider,FluoSpinbox>(*sizeLimitSlider,*sizeLimitSpinbox);
};

#endif
