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


  private:
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

    FluoSpinboxDouble* gammaSpinbox      = new FluoSpinboxDouble(0.0,1.0,false);
    FluoSpinboxDouble* extractBSpinbox   = new FluoSpinboxDouble(0.0,0.5,false);
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
    FluoToolButton* shadowLabel   = new FluoToolButton("Shadow: ",false);
    FluoToolButton* alphaLabel    = new FluoToolButton(" :Alpha",false);
    QLabel* sampleRateLabel       = new QLabel("Sample Rate: ");
    FluoToolButton* shadingLabel  = new FluoToolButton(" :Shading",false);
    FluoToolButton* colorMapLabel = new FluoToolButton("Color Map: ",false);

    const std::vector<std::function<void()>> rowFuncs = {
      std::bind(&VolumePropertiesOptions::addRow0,this),
      std::bind(&VolumePropertiesOptions::addRow1,this),
      std::bind(&VolumePropertiesOptions::addRow2,this),
      std::bind(&VolumePropertiesOptions::addRow3,this),
      std::bind(&VolumePropertiesOptions::addRow4,this),
    };
};

#endif
