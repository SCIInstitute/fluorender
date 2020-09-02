#ifndef VOLUME_PROPERTIES_OPTIONS
#define VOLUME_PROPERTIES_OPTIONS

#include <QGridLayout>
#include <QLabel>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoToolButton.hpp>
#include <CustomWidgets/controller.hpp>
#include <CustomWidgets/agentWrapper.hpp>

#include <Panels/Properties/Agent/volumePropAgent.hpp>

#include <vector>
#include <tuple>
#include <functional>

class VolumePropertiesOptions : public QGridLayout
{
  Q_OBJECT

  signals:

    void sendGammaValue(int value);
    void sendGammaValue(double value);

    void sendExtBoundValue(int value);
    void sendExtBoundValue(double value);

    void sendSaturationValue(int value);
    void sendLowThreshValue(int value);
    void sendHighThreshValue(int value);
    void sendLuminanceValue(int value);

    void sendShadowValue(int value);
    void sendShadowValue(double value);

    void sendAlphaValue(int value);

    void sendSampleValue(int value);
    void sendSampleValue(double value);

    void sendLowShadeValue(int value);
    void sendLowShadeValue(double value);

    void sendHighShadeValue(int value);
    void sendHighShadeValue(double value);

    void sendColorMapLowValue(int value);
    void sendColorMapHighValue(int value);

  public slots:
    void onGammaSliderChanged() { sendGammaValue(gammaSlider->value()); }
    void onGammaSpinboxChanged() { sendGammaValue(gammaSpinbox->value()); }

    void onExtBoundSliderChanged() { sendExtBoundValue(extractBSlider->value()); }
    void onExtBoundSpinboxChanged() { sendExtBoundValue(extractBSpinbox->value()); }

    void onSatSliderChanged() { sendSaturationValue(saturationSlider->value()); }
    void onSatSpinboxChanged() { sendSaturationValue(saturationSpinbox->value()); }

    void onLowThreshSliderChanged() { sendLowThreshValue(threshold1Slider->value()); }
    void onLowThreshSpinChanged() { sendLowThreshValue(threshold1Spinbox->value()); }
    void onHighThreshSliderChanged() { sendHighThreshValue(threshold2Slider->value()); }
    void onHighThreshSpinChanged() { sendHighThreshValue(threshold2Spinbox->value()); }

    void onLuminanceSliderChanged() { sendLuminanceValue(luminanceSlider->value()); }
    void onLuminanceSpinChanged() { sendLuminanceValue(luminanceSpinbox->value()); }

    void onShadowSliderChanged() { sendShadowValue(shadowSlider->value()); }
    void onShadowSpinChanged() { sendShadowValue(shadowSpinbox->value()); }

    void onAlphaSliderChanged() { sendAlphaValue(alphaSlider->value()); }
    void onAlphaSpinChanged() { sendAlphaValue(alphaSpinbox->value()); }

    void onSampleSliderChanged() { sendSampleValue(sampleRateSlider->value()); }
    void onSampleSpinChanged() { sendSampleValue(sampleRateSpinbox->value()); }

    void onLShaderSliderChanged() { sendLowShadeValue(shading1Slider->value()); }
    void onLShaderSpinChanged() { sendLowShadeValue(shading1Spinbox->value()); }
    void onHShaderSliderChanged() { sendHighShadeValue(shading2Slider->value()); }
    void onHShaderSpinChanged() { sendHighShadeValue(shading2Spinbox->value()); }

    void onLCMSliderChanged() { sendColorMapLowValue(colorMap1Slider->value()); }
    void onLCMSpinChanged() { sendColorMapLowValue(colorMap1Spinbox->value()); }
    void onHCMSliderChanged() { sendColorMapHighValue(colorMap2Slider->value()); }
    void onHCMSpinChanged() { sendColorMapHighValue(colorMap2Slider->value()); }

  public:

    VolumePropertiesOptions(VolumePropAgent *agent, fluo::VolumeData *vd);

    void setMaxVal(double newVal) { maxVal = newVal; }
    double getMaxVal() const { return maxVal; }

    template<typename T>
    void setGammaValue(T newVal) { gammaController->setValues(newVal); }

    template<typename T>
    void setExtBoundValue(T newVal) { boundaryController->setValues(newVal); }

    void setSaturationVal(int newVal) { saturationController->setValues(newVal); double a; m_agent->getValue("gamma 3d",a); std::cout << a << std::endl;}

    void setLowThreshValue(int newVal) { threshold1Controller->setValues(newVal); }
    void setHighThreshValue(int newVal) { threshold2Controller->setValues(newVal); }

    void setLuminanceVal(int newVal) { luminanceController->setValues(newVal); }

    void setShadowEnabled(bool status) { shadowLabel->setEnabled(status); }

    template<typename T>
    void setShadowValue(T newVal) { shadowController->setValues(newVal); }

    void setAlphaEnabled(bool status) { alphaLabel->setEnabled(status); }
    void setAlphaVal(int newVal) { alphaController->setValues(newVal); }

    template<typename T>
    void setSampleValue(T newVal) { sampleRateController->setValues(newVal); }

    void setShaderEnabled(bool status) { shadingLabel->setEnabled(status); }

    template<typename T>
    void setLowShaderVal(T newVal) { shading1Controller->setValues(newVal); }

    template<typename T>
    void setHighShaderVal(T newVal) { shading2Controller->setValues(newVal); }

    void setColorMapEnabled(bool status) { colorMapLabel->setEnabled(status); }
    void setColorMapLowVal(int newVal)  { colorMap1Controller->setValues(newVal); }
    void setColorMapHighVal(int newVal) { colorMap2Controller->setValues(newVal); }



  private:

    VolumePropAgent *m_agent;
    //friend class VolumePropAgent;

    typedef void(VolumePropertiesOptions::*volumeFunc)();
    typedef void(FluoSlider::*sliderFunc)(int);
    typedef void(FluoSpinbox::*spinFunc)();
    typedef void(FluoSpinboxDouble::*dSpinFunc)();

    double maxVal = 255.0;

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();

    void constructLayout();

    void buildSliderConnections();
    void buildSpinboxConnections();
    void buildSpinboxDConnections();
    void initializeWrappers();

    FluoSlider* gammaSlider      = new FluoSlider(Qt::Horizontal,0,1000);
    FluoSlider* extractBSlider   = new FluoSlider(Qt::Horizontal,0,100);
    FluoSlider* saturationSlider = new FluoSlider(Qt::Horizontal,0,255);
    FluoSlider* threshold1Slider = new FluoSlider(Qt::Horizontal,0,255);
    FluoSlider* threshold2Slider = new FluoSlider(Qt::Horizontal,0,255);
    FluoSlider* luminanceSlider  = new FluoSlider(Qt::Horizontal,0,255);
    FluoSlider* shadowSlider     = new FluoSlider(Qt::Horizontal,0,100);
    FluoSlider* alphaSlider      = new FluoSlider(Qt::Horizontal,0,255);
    FluoSlider* sampleRateSlider = new FluoSlider(Qt::Horizontal,10,500);
    FluoSlider* shading1Slider   = new FluoSlider(Qt::Horizontal,0,1000);
    FluoSlider* shading2Slider   = new FluoSlider(Qt::Horizontal,0,200);
    FluoSlider* colorMap1Slider  = new FluoSlider(Qt::Horizontal,0,255);
    FluoSlider* colorMap2Slider  = new FluoSlider(Qt::Horizontal,0,255);

    FluoSpinboxDouble* gammaSpinbox      = new FluoSpinboxDouble(0.0,10.0,false);
    FluoSpinboxDouble* extractBSpinbox   = new FluoSpinboxDouble(0.0,1.0,false); //TODO: Set precision to 4 decimals
    FluoSpinbox* saturationSpinbox       = new FluoSpinbox(0,255,false);
    FluoSpinbox* threshold1Spinbox       = new FluoSpinbox(0,255,false);
    FluoSpinbox* threshold2Spinbox       = new FluoSpinbox(0,255,false);
    FluoSpinbox* luminanceSpinbox        = new FluoSpinbox(0,255,false);
    FluoSpinboxDouble* shadowSpinbox     = new FluoSpinboxDouble(0.0,1.0,false);
    FluoSpinbox* alphaSpinbox            = new FluoSpinbox(0,255,false);
    FluoSpinboxDouble* sampleRateSpinbox = new FluoSpinboxDouble(0.1,5.0,false);
    FluoSpinboxDouble* shading1Spinbox   = new FluoSpinboxDouble(0.0,10.0,false);
    FluoSpinboxDouble* shading2Spinbox   = new FluoSpinboxDouble(0.0,2.0,false);
    FluoSpinbox* colorMap1Spinbox        = new FluoSpinbox(0,255,false);
    FluoSpinbox* colorMap2Spinbox        = new FluoSpinbox(0,255,false);

    QLabel* gammaLabel            = new QLabel(" :Gamma");
    QLabel* extractBLabel         = new QLabel("Extract Boundary: ");
    QLabel* saturationsLabel      = new QLabel(" :Saturation");
    QLabel* thresholdLabel        = new QLabel("Threshold: ");
    QLabel* luminanceLabel        = new QLabel(" :Luminance");
    FluoToolButton* shadowLabel   = new FluoToolButton("Shadow: ",true,true,false);
    FluoToolButton* alphaLabel    = new FluoToolButton(" :Alpha",true,true,false);
    QLabel* sampleRateLabel       = new QLabel("Sample Rate: ");
    FluoToolButton* shadingLabel  = new FluoToolButton(" :Shading",true,true,false);
    FluoToolButton* colorMapLabel = new FluoToolButton("Color Map: ",true,true,false);

    AgentWrapper<VolumePropAgent> *gammaWrapper = new AgentWrapper<VolumePropAgent>("gamma 3d", m_agent);
    AgentWrapper<VolumePropAgent> *extractWrapper = new AgentWrapper<VolumePropAgent>("extract boundary", m_agent);
    AgentWrapper<VolumePropAgent> *saturationWrapper = new AgentWrapper<VolumePropAgent>("saturation", m_agent);
    AgentWrapper<VolumePropAgent> *threshhold1Wrapper = new AgentWrapper<VolumePropAgent>("low threshold", m_agent);
    AgentWrapper<VolumePropAgent> *threshhold2Wrapper = new AgentWrapper<VolumePropAgent>("high threshold", m_agent);
    AgentWrapper<VolumePropAgent> *luminanceWrapper = new AgentWrapper<VolumePropAgent>("luminance", m_agent);
    AgentWrapper<VolumePropAgent> *shadowWrapper = new AgentWrapper<VolumePropAgent>("shadow int", m_agent);
    AgentWrapper<VolumePropAgent> *alphaWrapper = new AgentWrapper<VolumePropAgent>("alpha", m_agent);
    AgentWrapper<VolumePropAgent> *sampleWrapper = new AgentWrapper<VolumePropAgent>("sample rate", m_agent);
    AgentWrapper<VolumePropAgent> *shading1Wrapper = new AgentWrapper<VolumePropAgent>("mat amb", m_agent);
    AgentWrapper<VolumePropAgent> *shading2Wrapper = new AgentWrapper<VolumePropAgent>("mat shine", m_agent);
    AgentWrapper<VolumePropAgent> *colorMap1Wrapper = new AgentWrapper<VolumePropAgent>("colormap low", m_agent);
    AgentWrapper<VolumePropAgent> *colorMap2Wrapper = new AgentWrapper<VolumePropAgent>("colormap high", m_agent);

    const std::vector<std::tuple<FluoSlider*, sliderFunc, volumeFunc>> sliderConnections = {
      std::make_tuple(gammaSlider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onGammaSliderChanged),
      std::make_tuple(extractBSlider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onExtBoundSliderChanged),
      std::make_tuple(saturationSlider, &FluoSlider::valueChanged,&VolumePropertiesOptions::onSatSliderChanged),
      std::make_tuple(threshold1Slider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onLowThreshSliderChanged),
      std::make_tuple(threshold2Slider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onHighThreshSliderChanged),
      std::make_tuple(luminanceSlider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onLuminanceSliderChanged),
      std::make_tuple(shadowSlider, &FluoSlider::valueChanged,&VolumePropertiesOptions::onShadowSliderChanged),
      std::make_tuple(alphaSlider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onAlphaSliderChanged),
      std::make_tuple(sampleRateSlider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onSampleSliderChanged),
      std::make_tuple(shading1Slider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onLShaderSliderChanged),
      std::make_tuple(shading2Slider, &FluoSlider::valueChanged,&VolumePropertiesOptions::onHShaderSliderChanged),
      std::make_tuple(colorMap1Slider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onLCMSliderChanged),
      std::make_tuple(colorMap2Slider, &FluoSlider::valueChanged, &VolumePropertiesOptions::onHCMSliderChanged)
    };

    const std::vector<std::tuple<FluoSpinbox*, spinFunc, volumeFunc>> spinConnections = {
      std::make_tuple(saturationSpinbox, &FluoSpinbox::editingFinished,&VolumePropertiesOptions::onSatSpinboxChanged),
      std::make_tuple(threshold1Spinbox, &FluoSpinbox::editingFinished, &VolumePropertiesOptions::onLowThreshSpinChanged),
      std::make_tuple(threshold2Spinbox, &FluoSpinbox::editingFinished, &VolumePropertiesOptions::onHighThreshSpinChanged),
      std::make_tuple(luminanceSpinbox, &FluoSpinbox::editingFinished, &VolumePropertiesOptions::onLuminanceSpinChanged),
      std::make_tuple(alphaSpinbox, &FluoSpinbox::editingFinished, &VolumePropertiesOptions::onAlphaSpinChanged),
      std::make_tuple(colorMap1Spinbox, &FluoSpinbox::editingFinished, &VolumePropertiesOptions::onLCMSpinChanged),
      std::make_tuple(colorMap2Spinbox, &FluoSpinbox::editingFinished, &VolumePropertiesOptions::onHCMSpinChanged)
    };

    const std::vector<std::tuple<FluoSpinboxDouble*, dSpinFunc, volumeFunc>> dSpinConnections = {
      std::make_tuple(gammaSpinbox, &FluoSpinboxDouble::editingFinished, &VolumePropertiesOptions::onGammaSpinboxChanged),
      std::make_tuple(extractBSpinbox, &FluoSpinboxDouble::editingFinished, &VolumePropertiesOptions::onExtBoundSpinboxChanged),
      std::make_tuple(shadowSpinbox, &FluoSpinboxDouble::editingFinished,&VolumePropertiesOptions::onShadowSpinChanged),
      std::make_tuple(sampleRateSpinbox, &FluoSpinboxDouble::editingFinished, &VolumePropertiesOptions::onSampleSpinChanged),
      std::make_tuple(shading1Spinbox, &FluoSpinboxDouble::editingFinished, &VolumePropertiesOptions::onLShaderSpinChanged),
      std::make_tuple(shading2Spinbox, &FluoSpinboxDouble::editingFinished,&VolumePropertiesOptions::onHShaderSpinChanged)
    };

    const std::vector<std::function<void()>> rowFuncs = {
      std::bind(&VolumePropertiesOptions::addRow0,this),
      std::bind(&VolumePropertiesOptions::addRow1,this),
      std::bind(&VolumePropertiesOptions::addRow2,this),
      std::bind(&VolumePropertiesOptions::addRow3,this),
      std::bind(&VolumePropertiesOptions::addRow4,this),
    };

    Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>> *gammaController =
            new Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>>(*gammaSlider,*gammaSpinbox,*gammaWrapper);
    Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>> *boundaryController =
            new Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>>(*extractBSlider,*extractBSpinbox,*extractWrapper);
    Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>> *saturationController =
            new Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>>(*saturationSlider,*saturationSpinbox,*saturationWrapper);
    Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>> *threshold1Controller =
            new Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>>(*threshold1Slider,*threshold1Spinbox,*threshhold1Wrapper);
    Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>> *threshold2Controller =
            new Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>>(*threshold2Slider,*threshold2Spinbox,*threshhold2Wrapper);
    Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>> *luminanceController =
            new Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>>(*luminanceSlider,*luminanceSpinbox,*luminanceWrapper);
    Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>> *shadowController =
            new Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>>(*shadowSlider,*shadowSpinbox,*shadowWrapper);
    Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>> *alphaController =
            new Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>>(*alphaSlider,*alphaSpinbox,*alphaWrapper);
    Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>> *sampleRateController =
            new Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>>(*sampleRateSlider,*sampleRateSpinbox,*sampleWrapper);
    Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>> *shading1Controller =
            new Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>>(*shading1Slider,*shading1Spinbox,*shading1Wrapper);
    Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>> *shading2Controller =
            new Controller<FluoSlider,FluoSpinboxDouble,AgentWrapper<VolumePropAgent>>(*shading2Slider,*shading2Spinbox,*shading2Wrapper);
    Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>> *colorMap1Controller =
            new Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>>(*colorMap1Slider,*colorMap1Spinbox,*colorMap1Wrapper);
    Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>> *colorMap2Controller =
            new Controller<FluoSlider,FluoSpinbox,AgentWrapper<VolumePropAgent>>(*colorMap2Slider,*colorMap2Spinbox,*colorMap2Wrapper);
};

#endif
