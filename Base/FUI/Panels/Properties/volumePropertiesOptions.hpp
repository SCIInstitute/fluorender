#ifndef VOLUME_PROPERTIES_OPTIONS
#define VOLUME_PROPERTIES_OPTIONS

#include <QGridLayout>
#include <QLabel>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoToolButton.hpp>
#include <CustomWidgets/controller.hpp>

#include <vector>
#include <functional>

class VolumePropertiesOptions : public QGridLayout
{
  Q_OBJECT

  public slots:
    void onGammaSliderChanged() { setGammaValue(gammaSlider->value()); }
    void onGammaSpinboxChanged() { setGammaValue(gammaSpinbox->value()); }

    void onExtBoundSliderChanged() { setExtBoundValue(extractBSlider->value()); }
    void onExtBoundSpinboxChanged() { setExtBoundValue(extractBSpinbox->value()); }

    void onSatSliderChanged() { setSaturationVal(saturationSlider->value()); }
    void onSatSpinboxChanged() { setSaturationVal(saturationSpinbox->value()); }

    void onLowThreshSliderChanged() { setLowThreshValue(threshold1Slider->value()); }
    void onLowThreshSpinChanged() { setLowThreshValue(threshold1Spinbox->value()); }
    void onHighThreshSliderChanged() { setHighThreshValue(threshold2Slider->value()); }
    void onHighThreshSpinChanged() { setHighThreshValue(threshold2Spinbox->value()); }

    void onLuminanceSliderChanged() { setLuminanceVal(luminanceSlider->value()); }
    void onLuminanceSpinChanged() { setLuminanceVal(luminanceSpinbox->value()); }

    void onShadowSliderChanged() { setShadowValue(shadowSlider->value()); }
    void onShadowSpinChanged() { setShadowValue(shadowSpinbox->value()); }

    void onAlphaSliderChanged() { setAlphaVal(alphaSlider->value()); }
    void onAlphaSpinChanged() { setAlphaVal(alphaSpinbox->value()); }

    void onSampleSliderChanged() { setSampleValue(sampleRateSlider->value()); }
    void onSampleSpinChanged() { setSampleValue(sampleRateSpinbox->value()); }

    void onLShaderSliderChanged() { setLowShaderVal(shading1Slider->value()); }
    void onLShaderSpinChanged() { setLowShaderVal(shading1Spinbox->value()); }
    void onHShaderSliderChanged() { setHighShaderVal(shading2Slider->value()); }
    void onHShaderSpinChanged() { setHighShaderVal(shading2Spinbox->value()); }

    void onLCMSliderChanged() { setColorMapLowVal(colorMap1Slider->value()); }
    void onLCMSpinChanged() { setColorMapLowVal(colorMap1Spinbox->value()); }
    void onHCMSliderChanged() { setColorMapHighVal(colorMap2Slider->value()); }
    void onHCMSpinChanged() { setColorMapHighVal(colorMap2Slider->value()); }

  public:
    VolumePropertiesOptions();

    void setMaxVal(double newVal) { maxVal = newVal; }
    double getMaxVal() const { return maxVal; }

    template<typename T>
    void setGammaValue(T newVal) { gammaController->setValues(newVal); }

    template<typename T>
    void setExtBoundValue(T newVal) { boundaryController->setValues(newVal); }

    void setSaturationVal(int newVal) { saturationController->setValues(newVal); }

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

    double maxVal = 255.0;

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();

    void constructLayout();

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

    const std::vector<std::function<void()>> rowFuncs = {
      std::bind(&VolumePropertiesOptions::addRow0,this),
      std::bind(&VolumePropertiesOptions::addRow1,this),
      std::bind(&VolumePropertiesOptions::addRow2,this),
      std::bind(&VolumePropertiesOptions::addRow3,this),
      std::bind(&VolumePropertiesOptions::addRow4,this),
    };

    Controller<FluoSlider,FluoSpinboxDouble> *gammaController =
            new Controller<FluoSlider,FluoSpinboxDouble>(*gammaSlider,*gammaSpinbox);
    Controller<FluoSlider,FluoSpinboxDouble> *boundaryController =
            new Controller<FluoSlider,FluoSpinboxDouble>(*extractBSlider,*extractBSpinbox);
    Controller<FluoSlider,FluoSpinbox> *saturationController =
            new Controller<FluoSlider,FluoSpinbox>(*saturationSlider,*saturationSpinbox);
    Controller<FluoSlider,FluoSpinbox> *threshold1Controller =
            new Controller<FluoSlider,FluoSpinbox>(*threshold1Slider,*threshold1Spinbox);
    Controller<FluoSlider,FluoSpinbox> *threshold2Controller =
            new Controller<FluoSlider,FluoSpinbox>(*threshold2Slider,*threshold2Spinbox);
    Controller<FluoSlider,FluoSpinbox> *luminanceController =
            new Controller<FluoSlider,FluoSpinbox>(*luminanceSlider,*luminanceSpinbox);
    Controller<FluoSlider,FluoSpinboxDouble> *shadowController =
            new Controller<FluoSlider,FluoSpinboxDouble>(*shadowSlider,*shadowSpinbox);
    Controller<FluoSlider,FluoSpinbox> *alphaController =
            new Controller<FluoSlider,FluoSpinbox>(*alphaSlider,*alphaSpinbox);
    Controller<FluoSlider,FluoSpinboxDouble> *sampleRateController =
            new Controller<FluoSlider,FluoSpinboxDouble>(*sampleRateSlider,*sampleRateSpinbox);
    Controller<FluoSlider,FluoSpinboxDouble> *shading1Controller =
            new Controller<FluoSlider,FluoSpinboxDouble>(*shading1Slider,*shading1Spinbox);
    Controller<FluoSlider,FluoSpinboxDouble> *shading2Controller =
            new Controller<FluoSlider,FluoSpinboxDouble>(*shading2Slider,*shading2Spinbox);
    Controller<FluoSlider,FluoSpinbox> *colorMap1Controller =
            new Controller<FluoSlider,FluoSpinbox>(*colorMap1Slider,*colorMap1Spinbox);
    Controller<FluoSlider,FluoSpinbox> *colorMap2Controller =
            new Controller<FluoSlider,FluoSpinbox>(*colorMap2Slider,*colorMap2Spinbox);
};

#endif
