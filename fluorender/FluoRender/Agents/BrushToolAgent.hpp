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
class TreePanel;
namespace fluo
{
	class AgentFactory;
	class BrushToolAgent : public InterfaceAgent
	{
	public:
		BrushToolAgent(BrushToolDlg &dlg, TreePanel &panel);

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
		//brush ops
		void BrushClear();
		void BrushErase();
		void BrushCreate();
		void MaskCopy();
		void MaskCopyData();
		void MaskPaste(int op);
		//selector properties (move to selector once it becomes an object)
		void SetBrushSclTranslate(double v);
		void SetBrushGmFalloff(double v);
		void SetW2d(double v);
		void SetEdgeDetect(bool v);
		void SetHiddenRemoval(bool v);
		void SetSelectGroup(bool v);
		void SetEstimateThreshold(bool v);
		void SetUpdateOrder(bool v);
		void SetBrushSize(double v1, double v2);
		void SetUseBrushSize2(bool v);
		void SetBrushIteration(int v);
		void SetBrushSizeData(bool v);
		//other operations
		void AlignPca(int type, bool ac);
		//output
		void Update(int mode);//mode: 0-size; 1-speed
		void UpdateSize();
		void UpdateSpeed();

		virtual BrushToolAgent* asBrushToolAgent() { return this; }
		virtual const BrushToolAgent* asBrushToolAgent() const { return this; }

		friend class AgentFactory;

	protected:
		BrushToolDlg &dlg_;
		TreePanel &tree_panel_;//secondary ui

	private:
		//update functions
		void OnInterModeChanged(Event& event);
		void OnSelUndo(Event& event);
		void OnSelRedo(Event& event);
	};
}

#endif//_BRUSHTOOLAGENT_H_