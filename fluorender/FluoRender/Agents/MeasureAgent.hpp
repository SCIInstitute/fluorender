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
#ifndef _MEASUREAGENT_H_
#define _MEASUREAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>

class MeasureDlg;
namespace flrd
{
	class DistCalculator;
	class Ruler;
	class RulerList;
	class RulerAlign;
	class RulerHandler;
}
namespace fluo
{
	class MeasureAgent : public InterfaceAgent
	{
	public:
		MeasureAgent(MeasureDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const MeasureAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "MeasureAgent"; }

		virtual void setObject(Renderview* view);
		virtual Renderview* getObject();

		virtual void UpdateAllSettings();

		virtual MeasureAgent* asMeasureAgent() { return this; }
		virtual const MeasureAgent* asMeasureAgent() const { return this; }

		friend class AgentFactory;

		void UpdateRulers();
		//processing
		void Relax();
		void Relax(int idx);
		void Project(int idx);
		void Prune(int len);//remove branches with length equal to or smaller than len
		void Prune(int idx, int len);
		void AlignRuler();
		void AlignPca();
		void AlignCenter(flrd::Ruler* ruler, flrd::RulerList* ruler_list);
		//list
		void SelectGroup(unsigned int group);
		void DeleteSelection();
		void DeleteAll(bool cur_time);
		void Export(const std::string &filename);
		void EndEdit(bool update);
		//ruler on list
		void SelectRuler(long i);
		Point GetRulerCenter(unsigned int i);
		flrd::Ruler* GetRuler(unsigned int i);
		void SetRulerName(unsigned int i, const std::string &name);
		void SetRulerCenter(unsigned int i, const Point &p);
		void SetRulerColor(unsigned int i, const Color &c);
		bool ToggleRulerDisp(unsigned int i);
		//ruler tools
		void ToggleToolBtns(bool val);
		void Locator();
		void Probe();
		void Protractor();
		void Ruler();
		void RulerMP();
		void Ellipse();
		void Grow();
		void Pencil();
		//modify
		void RulerFlip();
		void RulerEdit();
		void RulerDelete();
		void RulerMove();
		void RulerAvg();
		void Lock();
		//analysis
		void Profile();
		void Distance(const std::string& filename);
		void Project();
		//group
		void NewGroup();
		void ChangeGroup(unsigned int ival);
		void ToggleGroupDisp(unsigned int ival);

	protected:
		MeasureDlg &dlg_;

	protected:
		virtual ~MeasureAgent();

	private:
		flrd::DistCalculator* m_calculator;

	private:
		//update functions
		void OnRulerF1(Event& event);
	};
}

#endif//_MEASUREAGENT_H_