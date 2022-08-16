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
#ifndef _COMPONENTAGENT_H_
#define _COMPONENTAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>
#include <CompGenerator.h>
#include <chrono>
#include <set>

class ComponentDlg;
namespace fluo
{
	class AgentFactory;
	class ComponentAgent : public InterfaceAgent
	{
	public:
		ComponentAgent(ComponentDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const ComponentAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "ComponentAgent"; }

		virtual void setObject(Renderview* obj);
		virtual Renderview* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual ComponentAgent* asComponentAgent() { return this; }
		virtual const ComponentAgent* asComponentAgent() const { return this; }

		friend class AgentFactory;

		void LoadSettings(const std::string &filename);
		void SaveSettings(const std::string &filename);
		void Analyze();
		void Analyze(bool use_sel);
		void GenerateComp();
		void GenerateComp(bool use_selection, bool command);
		void Fixate(bool command);
		void Clean();
		void Clean(bool use_sel, bool command = true);
		void CompFull();
		void CompAppend();
		void CompExclusive();
		void CompAll();
		void CompClear();
		void CompNew();
		void CompAdd();
		void CompReplace();
		void CompCleanBkg();
		void CompCombine();

		//command
		void LoadCmd(const std::string &filename);
		void SaveCmd(const std::string &filename);
		void AddCmd(const std::string &type);
		void ResetCmd();
		void PlayCmd(bool use_selection, double tfactor);

		//cluster
		void Cluster();

		//shuffle value
		void ShuffleCurVolume();
		int GetShuffle();

		//output
		void CompOutput(int color_type);
		void OutputMulti(int color_type);
		void OutputRgb(int color_type);
		void OutputAnnotation(int type);

		//dist
		int GetDistMatSize();
		void DistOutput(const std::wstring &filename);

		//align
		void AlignCenter(flrd::Ruler* ruler);
		void AlignPca();

		//select comps
		bool GetCellList(flrd::CelpList &cl, bool links = false);
		void GetCompSelection();
		void SetCompSelection(std::set<unsigned long long>& ids, int mode);
		void IncludeComps();
		void ExcludeComps();

		//in and out cell lists
		flrd::CelpList &GetInCells()
		{
			return m_in_cells;
		}
		flrd::CelpList &GetOutCells()
		{
			return m_out_cells;
		}

	protected:
		ComponentDlg &dlg_;

		virtual void setupInputs();

	private:
		//speed test
		std::vector<std::chrono::high_resolution_clock::time_point> m_tps;

		flrd::CompCommand m_command;
		//in and out cell lists for tracking
		flrd::CelpList m_in_cells;
		flrd::CelpList m_out_cells;

	private:
		//speed test
		void StartTimer();
		void StopTimer(std::string &values, const std::string &str);

		bool GetIds(std::string &str, unsigned int &id, int &brick_id);
		void FindCelps(flrd::CelpList &list,
			flrd::CelpListIter &it, bool links = false);

		//update functions
		void OnAutoUpdate(Event& event);
		void OnUseDistField(Event& event);
		void OnUseDiffusion(Event& event);
		void OnUseDensityField(Event& event);
		void OnFixateEnable(Event& event);
		void OnFixateSize(Event& event);
		void OnCleanEnable(Event& event);
		void OnClusterMethod(Event& event);
		void OnUseMin(Event& event);
		void OnUseMax(Event& event);
	};
}

#endif//_COMPONENTAGENT_H_