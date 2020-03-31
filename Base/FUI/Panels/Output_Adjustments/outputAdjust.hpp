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

    void onRedGammaReceived(std::any value) { setOutRedGammaValue(value); }
    void onGreenGammaReceived(std::any value) { setOutGreenGammaValue(value); }
    void onBlueGammaReceived(std::any value) { setOutBlueGammaValue(value); }
    
    void onRedLuminReceived(int value) { setOutRedLuminValue(value); }
    void onGreenLuminReceived(int value) { setOutGreenLuminValue(value); }
    void onBlueLuminReceived(int value) { setOutBlueLuminValue(value); }
  
    void onRedEqlReceived(std::any value) { setOutRedEqlValue(value); }
    void onGreenEqlReceived(std::any value) { setOutGreenEqlValue(value); }
    void onBlueEqlReceived(std::any value) { setOutBlueEqlValue(value); }

  public:
    OutputAdjustments();

    template<typename T>
    void setOutRedGammaValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setRedGammaValue(newVal);
    }

    template<typename T>
    void setOutGreenGammaValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setGreenGammaValue(newVal);
    }

    template<typename T>
    void setOutBlueGammaValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setBlueGammaValue(newVal);
    }

    void setOutRedLuminValue(int newVal);
    void setOutGreenLuminValue(int newVal);
    void setOutBlueLuminValue(int newVal);

    template<typename T>
    void setOutRedEqlValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setRedEqlValue(newVal);
    }

    template<typename T>
    void setOutGreenEqlValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setGreenEqlValue(newVal);
    }

    template<typename T>
    void setOutBlueEqlValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setBlueEqlValue(newVal);
    }

  private:

    typedef void(OutputAdjustments::*outputAnyFunc)(std::any);
    typedef void(OutputAdjustments::*outputIntFunc)(int);
    typedef void(OutputLayout::*layoutAnyFunc)(std::any);
    typedef void(OutputLayout::*layoutIntFunc)(int);

    QGroupBox *outputFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    OutputLayout *outputLayout = new OutputLayout();

    OutputLayout* getOutputLayout();

    void makeStdAnyConnections();
    void makeIntConnections();

    const std::vector<std::tuple<OutputLayout*, layoutAnyFunc, outputAnyFunc>> stdAnyConnections = {
      std::make_tuple(outputLayout,&OutputLayout::sendRedGammaValue,&OutputAdjustments::onRedGammaReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendGreenGammaValue,&OutputAdjustments::onGreenGammaReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendBlueGammaValue,&OutputAdjustments::onBlueGammaReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendRedEqlValue,&OutputAdjustments::onRedEqlReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendGreenEqlValue,&OutputAdjustments::onGreenEqlReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendBlueEqlValue,&OutputAdjustments::onBlueEqlReceived)
    };

    const std::vector<std::tuple<OutputLayout*, layoutIntFunc, outputIntFunc>> intConnections = {
      std::make_tuple(outputLayout,&OutputLayout::sendRedLuminValue,&OutputAdjustments::onRedLuminReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendGreenLuminValue,&OutputAdjustments::onGreenLuminReceived),
      std::make_tuple(outputLayout,&OutputLayout::sendBlueLuminValue,&OutputAdjustments::onBlueLuminReceived)
    };

    OutAdjustAgent* m_agent;
    friend class OutAdjustAgent;
};

#endif
