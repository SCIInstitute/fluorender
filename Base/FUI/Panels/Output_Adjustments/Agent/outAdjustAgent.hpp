/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#ifndef OUT_ADJUST_AGENT_HPP
#define OUT_ADJUST_AGENT_HPP

#include <AgentFactory.hpp>
#include <VolumeData/VolumeData.hpp>

class OutputAdjustments;
class AgentFactory;
class OutAdjustAgent : public InterfaceAgent
{
  public:
    OutAdjustAgent(OutputAdjustments *panel);

/*
    virtual bool isSameKindAs(const fluo::Object* obj) const
    {
      return dynamic_cast<const OutAdjustAgent*>(obj) != NULL;
    }
*/
    virtual const char* className() const { return "OutAdjustAgent"; }
  
    virtual void setObject(fluo::VolumeData* vd);
    virtual fluo::VolumeData* getObject();
  
    virtual void UpdateAllSettings();
  
    friend class AgentFactory;

  protected:

    //update functions
    void OnGammaRChanged(fluo::Event& event);
    void OnGammaGChanged(fluo::Event& event);
    void OnGammaBChanged(fluo::Event& event);
    void OnBrightnessRChanged(fluo::Event& event);
    void OnBrightnessGChanged(fluo::Event& event);
    void OnBrightnessBChanged(fluo::Event& event);
    void OnEqualizeRChanged(fluo::Event& event);
    void OnEqualizeGChanged(fluo::Event& event);
    void OnEqualizeBChanged(fluo::Event& event);

  private:
    OutputAdjustments *parentPanel;

    const int Gamma2UiS(double v) { return static_cast<int>(100.0/v+0.5); }
    const int Brigt2UiS(double v) { return static_cast<int>((v - 1.0) * 256.0 + 0.5); }
    const int Equal2UiS(double v) { return static_cast<int>(v * 100.0 + 0.5); }
    const double Gamma2UiT(double v) { return 1.0 / v; }
    const double Brigt2UiT(double v) { return (v - 1.0) * 256.0; }
};

#endif