/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef _OUTADJUSTAGENT_H_
#define _OUTADJUSTAGENT_H_

#include <InterfaceAgent.hpp>
#include <Node.hpp>

#define Gamma2UiS(v) \
	int(100.0/v+0.5)
#define Brigt2UiS(v) \
	int((v-1.0)*256.0+0.5)
#define Equal2UiS(v) \
	int(v*100.0+0.5)
#define Gamma2UiT(v) \
	1.0/v
#define Brigt2UiT(v) \
	(v-1.0)*256.0
#define Equal2UiT(v) \
	v

class OutAdjustPanel;
namespace fluo
{
	class AgentFactory;
	class OutAdjustAgent : public InterfaceAgent
	{
	public:
		OutAdjustAgent(OutAdjustPanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const OutAdjustAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "OutAdjustAgent"; }

		virtual void setObject(Node* vd);
		virtual Node* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual OutAdjustAgent* asOutAdjustAgent() { return this; }
		virtual const OutAdjustAgent* asOutAdjustAgent() const { return this; }

		friend class AgentFactory;

	protected:
		OutAdjustPanel &panel_;

		//update functions
		void OnGammaRChanged(Event& event);
		void OnGammaGChanged(Event& event);
		void OnGammaBChanged(Event& event);
		void OnBrightnessRChanged(Event& event);
		void OnBrightnessGChanged(Event& event);
		void OnBrightnessBChanged(Event& event);
		void OnEqualizeRChanged(Event& event);
		void OnEqualizeGChanged(Event& event);
		void OnEqualizeBChanged(Event& event);
	};
}

#endif//_OUTADJUSTAGENT_H_