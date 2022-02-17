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
#ifndef _COLOCALAGENT_H_
#define _COLOCALAGENT_H_

#include <Names.hpp>
#include <InterfaceAgent.hpp>
#include <VolumeGroup.hpp>
#include <limits>

class ColocalDlg;
namespace fluo
{
	class AgentFactory;
	class ColocalAgent : public InterfaceAgent
	{
	public:
		ColocalAgent(ColocalDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const ColocalAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "ColocalAgent"; }

		virtual void setObject(VolumeGroup*);
		virtual VolumeGroup* getObject();

		virtual void UpdateAllSettings();

		void Run();

		friend class AgentFactory;

	protected:
		ColocalDlg &dlg_;

	private:
		void SetOutput(const std::string &titles, const std::string &values);
		void ResetMinMax()
		{
			double dval = std::numeric_limits<double>::max();
			setValue(gstColormapLow, dval);
			setValue(gstColormapHigh, -dval);
		}
		void SetMinMax(double v)
		{
			double dval;
			getValue(gstColormapLow, dval);
			dval = std::min(v, dval);
			setValue(gstColormapLow, dval);
			getValue(gstColormapHigh, dval);
			dval = std::max(v, dval);
			setValue(gstColormapHigh, dval);
		}
	};
}

#endif//_COLOCALAGENT_H_