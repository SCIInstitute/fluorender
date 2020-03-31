#ifndef OUTPUT_ADJUSTMENTS_HPP
#define OUTPUT_ADJUSTMENTS_HPP

#include <QWidget>
#include <QGroupBox>

#include "outputLayout.hpp"

#include <Panels/Output_Adjustments/Agent/outAdjustAgent.hpp>

class OutputAdjustments : public QWidget
{
  Q_OBJECT
  
  public:
    OutputAdjustments();

    template<typename T>
    void setRedGammaValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setRedGammaValue(newVal);
    }

    template<typename T>
    void setGreenGammaValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setGreenGammaValue(newVal);
    }

    template<typename T>
    void setBlueGammaValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setGreenGammaValue(newVal);
    }

    void setRedLuminValue(int newVal);
    void setGreenLuminValue(int newVal);
    void setBlueLuminValue(int newVal);

    template<typename T>
    void setRedEqlValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setRedEqlValue(newVal);
    }

    template<typename T>
    void setGreenEqlValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setGreenEqlValue(newVal);
    }

    template<typename T>
    void setBlueEqlValue(T newVal)
    {
      OutputLayout* temp = getOutputLayout();
      temp->setGreenEqlValue(newVal);
    }

  private:

    QGroupBox *outputFrame = new QGroupBox();
    QGridLayout *frameLayout = new QGridLayout();
    OutputLayout *outputLayout = new OutputLayout();

    OutputLayout* getOutputLayout();

    OutAdjustAgent* m_agent;
    friend class OutAdjustAgent;


    
};

#endif
