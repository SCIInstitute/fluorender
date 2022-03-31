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
#ifndef _RECORDERAGENT_H_
#define _RECORDERAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>

class RecorderDlg;
namespace fluo
{
	class RecorderAgent : public InterfaceAgent
	{
	public:
		RecorderAgent(RecorderDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const RecorderAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "RecorderAgent"; }

		virtual void setObject(Renderview* view);
		virtual Renderview* getObject();

		virtual void UpdateAllSettings();

		virtual RecorderAgent* asRecorderAgent() { return this; }
		virtual const RecorderAgent* asRecorderAgent() const { return this; }

		friend class AgentFactory;

		void AutoKeyChanComb(int comb);
		void AddKey();
		void InsertKey(int index, double duration, int interpolation);
		bool MoveOne(std::vector<bool>& chan_mask, int lv = 1);
		void DeleteSel();
		void DeleteAll();

	protected:
		RecorderDlg &dlg_;

	private:
		void OnSelectedKey(Event& event);
	};
}

#endif//_RECORDERAGENT_H_