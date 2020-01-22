#ifndef VOLUME_PROPERTIES_OPTIONS
#define VOLUME_PROPERTIES_OPTIONS

#include <QGridLayout>
#include <QLabel>

#include <CustomWidgets/fluoSlider.hpp>
#include <CustomWidgets/fluoSpinbox.hpp>
#include <CustomWidgets/fluoSpinboxDouble.hpp>
#include <CustomWidgets/fluoToolButton.hpp>

#include <vector>
#include <functional>

class VolumePropertiesOptions : public QGridLayout
{
  Q_OBJECT

  public:
    VolumePropertiesOptions();

    void setMaxVal(double newVal) { maxVal = newVal; }
    double getMaxVal() const { return maxVal; }

    void setGammaSliderVal(int newVal) { gammaSlider->setValue(newVal); }
    void setGammaSpinboxVal(double newVal) { gammaSpinbox->setValue(newVal); }

    void setExtBounSliderVal(int newVal) { extractBSlider->setValue(newVal); }
    void setExtBounSpinBoxVal(double newVal) { extractBSpinbox->setValue(newVal); }

    void setSatSliderVal(int newVal) { saturationSlider->setValue(newVal); }
    void setSatSpinboxVal(int newVal) { saturationSpinbox->setValue(newVal); }

    void setLowThreshSliderVal(int newVal) { threshold1Slider->setValue(newVal); }
    void setLowThreshSpinboxVal(int newVal) { threshold1Spinbox->setValue(newVal); }
    void setHighThreSliderVal(int newVal) { threshold2Slider->setValue(newVal); }
    void setHighThreSpinboxVal(int newVal) { threshold2Spinbox->setValue(newVal); }

    void setLuminSliderVal(int newVal) { luminanceSlider->setValue(newVal); }
    void setLuminSpinboxVal(int newVal) { luminanceSpinbox->setValue(newVal); }

    void setShadowEnabled(bool status) { shadowLabel->setEnabled(status); }
    void setShadowSliderVal(int newVal) { shadowSlider->setValue(newVal); }
    void setShadowSpinboxVal(double newVal) { shadowSpinbox->setValue(newVal); }

    void setAlphaEnabled(bool status) { alphaLabel->setEnabled(status); }
    void setAlphaSliderVal(int newVal) { alphaSlider->setValue(newVal); }
    void setAlphaSpinboxVal(int newVal) { alphaSpinbox->setValue(newVal); }

    void setSampleSliderVal(int newVal) { sampleRateSlider->setValue(newVal); }
    void setSampleSpinboxVal(double newVal) { sampleRateSpinbox->setValue(newVal); }

    void setShaderEnabled(bool status) { shadingLabel->setEnabled(status); }
    void setLowShadeSliderVal(int newVal) { shading1Slider->setValue(newVal); }
    void setLowShadeSpinboxVal(double newVal) { shading1Spinbox->setValue(newVal); }
    void setHighShadeSliderVal(int newVal) { shading2Slider->setValue(newVal); }
    void setHighShadeSpinboxVal(double newVal) { shading2Spinbox->setValue(newVal); }

    void setColorMapEnabled(bool status) { colorMapLabel->setEnabled(status); }
    void setCMLowSliderVal(int newVal) { colorMap1Slider->setValue(newVal); }
    void setCMLowSpinboxVal(int newVal) { colorMap1Spinbox->setValue(newVal); }
    void setCMHighSliderVal(int newVal) { colorMap2Slider->setValue(newVal); }
    void setCMHighSpinboxVal(int newVal) { colorMap2Spinbox->setValue(newVal); }



  private:

    double maxVal = 255.0;

    void addRow0();
    void addRow1();
    void addRow2();
    void addRow3();
    void addRow4();

    void constructLayout();

    FluoSlider* gammaSlider      = new FluoSlider(Qt::Horizontal,0,100);
    FluoSlider* extractBSlider   = new FluoSlider(Qt::Horizontal,0,50);
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
};

#endif
