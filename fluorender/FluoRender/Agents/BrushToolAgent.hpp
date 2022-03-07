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
#ifndef _BRUSHTOOLAGENT_H_
#define _BRUSHTOOLAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>

#define GM_2_ESTR(x) (1.0 - sqrt(1.0 - (x - 1.0) * (x - 1.0)))

class BrushToolDlg;
namespace fluo
{
	class AgentFactory;
	class BrushToolAgent : public InterfaceAgent
	{
	public:
		BrushToolAgent(BrushToolDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const BrushToolAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "BrushToolAgent"; }

		virtual void setObject(Renderview* obj);
		virtual Renderview* getObject();

		virtual void UpdateAllSettings();
		//update undo status
		void UpdateUndoRedo();
		void UpdateMaskTb();

		virtual BrushToolAgent* asBrushToolAgent() { return this; }
		virtual const BrushToolAgent* asBrushToolAgent() const { return this; }

		friend class AgentFactory;

	protected:
		BrushToolDlg &dlg_;

	private:
		//update functions
		void OnInterModeChanged(Event& event);
	};
}

#endif//_BRUSHTOOLAGENT_H_