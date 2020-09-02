#ifndef OUTPUT_ADJUSTMENTS_HPP
#define OUTPUT_ADJUSTMENTS_HPP

#include <QWidget>
#include <QGroupBox>

#include "outputLayout.hpp"

#include <Panels/Output_Adjustments/Agent/outAdjustAgent.hpp>

class OutputAdjustments : public QWidget
{
  Q_OBJECT

  public slots:

    void onRedGammaReceived(int value) { setOutRedGammaValue(value); }
    void onRedGammaReceived(double value) { setOutRedGammaValue(value); }
    void onGreenGammaReceived(int value) { setOutGreenGammaValue(value); }
    void onGreenGammaReceived(double value) { setOutGreenGammaValue(value); }
    void onBlueGammaReceived(int value) { setOutBlueGammaValue(value); }
    void onBlueGammaReceived(double value) { setOutBlueGammaValue(value); }
    
    void onRedLuminReceived(int value) { setOutRedLuminValue(value); }
    void onGreenLuminReceived(int value) { setOutGreenLuminValue(value); }
    void onBlueLuminReceived(int value) { setOutBlueLuminValue(value); }
  
    void onRedEqlReceived(int value) { setOutRedEqlValue(value); }
    void onRedEqlReceived(double value) { setOutRedEqlValue(value); }
    void onGreenEqlReceived(int value) { setOutGreenEqlValue(value); }
    void onGreenEqlReceived(double value) { setOutGreenEqlValue(value); }
    void onBlueEqlReceived(int value) { setOutBlueEqlValue(value); }
    void onBlueEqlReceived(double value) { setOutBlueEqlValue(value); }

  public:
    OutputAdjustments();

    void setVolumeData(fluo::VolumeData *vd);

    template<typename T>
    void setOutRedGammaValue(T newVal)
    {
      OutputLayout* temp = this->findChild<OutputLayout*>();
      temp->setRedGammaValue(newVal);
    }

    template<typename T>
    void setOutGreenGammaValue(T newVal)
    {
      this->outputLayout->setGreenGammaValue(newVal);
    }

    template<typename T>
    void setOutBlueGammaValue(T newVal)
    {
      this->outputLayout->setBlueGammaValue(newVal);
    }

    void setOutRedLuminValue(int newVal);
    void setOutGreenLuminValue(int newVal);
    void setOutBlueLuminValue(int newVal);

    template<typename T>
    void setOutRedEqlValue(T newVal)
    {
      this->outputLayout->setRedEqlValue(newVal);
    }

    template<typename T>
    void setOutGreenEqlValue(T newVal)
    {
      this->outputLayout->setGreenEqlValue(newVal);
    }

    template<typename T>
    void setOutBlueEqlValue(T newVal)
    {
      this->outputLayout->setBlueEqlValue(newVal);
    }

  private:
    OutAdjustAgent* m_agent;
    friend class OutAdjustAgent;

    typedef void(OutputAdjustments::*outputIntFunc)(int);
    typedef void(OutputAdjustments::*outputDblFunc)(double);
    typedef void(OutputLayout::*layoutIntFunc)(int);
    typedef void(OutputLayout::*layoutDblFunc)(double);

    QGroupBox *outputFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    OutputLayout *outputLayout = new OutputLayout();

    void makeIntConnections();
    void makeDblConnections();

    const std::vector<std::tuple<OutputLayout*, layoutDblFunc, outputDblFunc>> dblConnections = {
      std::make_tuple(outputLayout,static_cast<layoutDblFunc>(&OutputLayout::sendRedGammaValue),
                      static_cast<outputDblFunc>(&OutputAdjustments::onRedGammaReceived)),
      std::make_tuple(outputLayout,static_cast<layoutDblFunc>(&OutputLayout::sendGreenGammaValue),
                      static_cast<outputDblFunc>(&OutputAdjustments::onGreenGammaReceived)),
      std::make_tuple(outputLayout,static_cast<layoutDblFunc>(&OutputLayout::sendBlueGammaValue),
                      static_cast<outputDblFunc>(&OutputAdjustments::onBlueGammaReceived)),
      std::make_tuple(outputLayout,static_cast<layoutDblFunc>(&OutputLayout::sendRedEqlValue),
                      static_cast<outputDblFunc>(&OutputAdjustments::onRedEqlReceived)),
      std::make_tuple(outputLayout,static_cast<layoutDblFunc>(&OutputLayout::sendGreenEqlValue),
                      static_cast<outputDblFunc>(&OutputAdjustments::onGreenEqlReceived)),
      std::make_tuple(outputLayout,static_cast<layoutDblFunc>(&OutputLayout::sendBlueEqlValue),
                      static_cast<outputDblFunc>(&OutputAdjustments::onBlueEqlReceived))
    };

    const std::vector<std::tuple<OutputLayout*, layoutIntFunc, outputIntFunc>> intConnections = {
      std::make_tuple(outputLayout,&OutputLayout::sendRedLuminValue,&OutputAdjustments::onRedLuminReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendGreenLuminValue,&OutputAdjustments::onGreenLuminReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendBlueLuminValue,&OutputAdjustments::onBlueLuminReceived),
      
      std::make_tuple(outputLayout,static_cast<layoutIntFunc>(&OutputLayout::sendRedGammaValue),
                      static_cast<outputIntFunc>(&OutputAdjustments::onRedGammaReceived)),
      std::make_tuple(outputLayout,static_cast<layoutIntFunc>(&OutputLayout::sendGreenGammaValue),
                      static_cast<outputIntFunc>(&OutputAdjustments::onGreenGammaReceived)),
      std::make_tuple(outputLayout,static_cast<layoutIntFunc>(&OutputLayout::sendBlueGammaValue),
                      static_cast<outputIntFunc>(&OutputAdjustments::onBlueGammaReceived)),
      std::make_tuple(outputLayout,static_cast<layoutIntFunc>(&OutputLayout::sendRedEqlValue),
                      static_cast<outputIntFunc>(&OutputAdjustments::onRedEqlReceived)),
      std::make_tuple(outputLayout,static_cast<layoutIntFunc>(&OutputLayout::sendGreenEqlValue),
                      static_cast<outputIntFunc>(&OutputAdjustments::onGreenEqlReceived)),
      std::make_tuple(outputLayout,static_cast<layoutIntFunc>(&OutputLayout::sendBlueEqlValue),
                      static_cast<outputIntFunc>(&OutputAdjustments::onBlueEqlReceived))
    };

};

#endif
