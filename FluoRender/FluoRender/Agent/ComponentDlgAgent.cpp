/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <ComponentDlgAgent.h>
#include <ComponentDlg.h>
#include <Global.h>
#include <Names.h>
#include <CompGenerator.h>
#include <CompAnalyzer.h>
#include <VolumeSelector.h>
#include <VolumeData.h>
#include <Clusterizer.h>
#include <CompEditor.h>
#include <CompSelector.h>
#include <RulerAlign.h>
#include <ModalDlg.h>
#include <CurrentObjects.h>
#include <ConvVolMesh.h>
#include <ColorMesh.h>
#include <DataManager.h>
#include <RenderView.h>
#include <MeshData.h>
#include <Coordinator.h>
#include <RulerList.h>
#include <compatibility.h>

ComponentDlgAgent::ComponentDlgAgent(
	ComponentDlg* dlg) :
	Agent(dlg)
{
	m_comp_gen_timer.setFunc([this]()
		{
			TimerGenerateComps();
		});
}

void ComponentDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	bool update_all = request.values.empty();

	int ival;
	double dval;
	bool bval;

	if (update_all || request.HasValue(gstUseSelection))
	{
		dlg->UpdateUseSelection(glbin_comp_generator.GetUseSel(),
			glbin_comp_analyzer.GetUseSel());
	}

	if (update_all || request.HasValue(gstUseMachineLearning))
	{
		dlg->UpdateUseMachineLearning(glbin_comp_generator.GetUseMl());
	}

	//comp generate page
	if (update_all || request.HasValue(gstIteration))
	{
		ival = glbin_comp_generator.GetIter();
		dlg->UpdateIteration(ival);
	}
	if (update_all || request.HasValue(gstCompThreshold))
	{
		dval = glbin_comp_generator.GetThresh();
		dlg->UpdateCompThreshold(dval);
	}
	//diffusion
	if (update_all || request.HasValue(gstUseDiffusion))
	{
		bval = glbin_comp_generator.GetDiffusion();
		dlg->UpdateDiffusion(bval);
	}
	if (update_all || request.HasValue(gstDiffusionFalloff))
	{
		dval = glbin_comp_generator.GetFalloff();
		dlg->UpdateDiffusionFalloff(dval);
	}
	//density
	if (update_all || request.HasValue(gstUseDensityField))
	{
		bval = glbin_comp_generator.GetDensity();
		dlg->UpdateUseDensityField(bval);
	}
	if (update_all || request.HasValue(gstDensityFieldThresh))
	{
		dval = glbin_comp_generator.GetDensityThresh();
		dlg->UpdateDensityFieldThresh(dval);
	}
	if (update_all || request.HasValue(gstDensityVarThresh))
	{
		dval = glbin_comp_generator.GetVarThresh();
		dlg->UpdateDensityVarThresh(dval);
	}
	if (update_all || request.HasValue(gstDensityWindowSize))
	{
		ival = glbin_comp_generator.GetDensityWinSize();
		dlg->UpdateDensityWindowSize(ival);
	}
	if (update_all || request.HasValue(gstDensityStatsSize))
	{
		ival = glbin_comp_generator.GetDensityStatSize();
		dlg->UpdateDensityStatsSize(ival);
	}
	//dist
	if (update_all || request.HasValue(gstUseDistField))
	{
		bval = glbin_comp_generator.GetUseDistField();
		dlg->UpdateUseDistField(bval);
	}
	if (update_all || request.HasValue(gstDistFieldStrength))
	{
		dval = glbin_comp_generator.GetDistStrength();
		dlg->UpdateDistFieldStrength(dval);
	}
	if (update_all || request.HasValue(gstDistFieldFilterSize))
	{
		ival = glbin_comp_generator.GetDistFilterSize();
		dlg->UpdateDistFieldFilterSize(ival);
	}
	if (update_all || request.HasValue(gstMaxDist))
	{
		ival = glbin_comp_generator.GetMaxDist();
		dlg->UpdateMaxDist(ival);
	}
	if (update_all || request.HasValue(gstDistFieldThresh))
	{
		dval = glbin_comp_generator.GetDistThresh();
		dlg->UpdateDistFieldThresh(dval);
	}

	//fixate
	if (update_all || request.HasValue(gstFixateEnable))
	{
		bval = glbin_comp_generator.GetFixate();
		dlg->UpdateFixateEnable(bval);
	}
	if (update_all || request.HasValue(gstGrowFixed))
	{
		bval = glbin_comp_generator.GetGrowFixed();
		dlg->UpdateGrowFixed(bval);
	}
	if (update_all || request.HasValue(gstFixateSize))
	{
		ival = glbin_comp_generator.GetFixSize();
		dlg->UpdateFixateSize(ival);
	}
	//clean
	if (update_all || request.HasValue(gstCleanEnable))
	{
		bval = glbin_comp_generator.GetClean();
		dlg->UpdateCleanEnable(bval);
	}
	if (update_all || request.HasValue(gstCleanIteration))
	{
		ival = glbin_comp_generator.GetCleanIter();
		dlg->UpdateCleanIteration(ival);
	}
	if (update_all || request.HasValue(gstCleanSize))
	{
		ival = glbin_comp_generator.GetCleanSize();
		dlg->UpdateCleanSize(ival);
	}
	//record
	if (update_all || request.HasValue(gstRecordCmd))
	{
		ival = glbin_comp_generator.GetCmdNum();
		bval = glbin_comp_generator.GetRecordCmd();
		dlg->UpdateRecordCmd(ival, bval);
	}

	//cluster page
	if (update_all || request.HasValue(gstClusterMethod))
	{
		ival = glbin_clusterizer.GetMethod();
		dlg->UpdateClusterMethod(ival);
	}
	//parameters
	if (update_all || request.HasValue(gstClusterNum))
	{
		ival = glbin_clusterizer.GetNum();
		dlg->UpdateClusterNum(ival);
	}
	if (update_all || request.HasValue(gstClusterMaxIter))
	{
		ival = glbin_clusterizer.GetMaxIter();
		dlg->UpdateClusterMaxIter(ival);
	}
	if (update_all || request.HasValue(gstClusterTol))
	{
		dval = glbin_clusterizer.GetTol();
		dlg->UpdateClusterTol(dval);
	}
	if (update_all || request.HasValue(gstClusterSize))
	{
		ival = glbin_clusterizer.GetSize();
		dlg->UpdateClusterSize(ival);
	}
	if (update_all || request.HasValue(gstClusterEps))
	{
		dval = glbin_clusterizer.GetEps();
		dlg->UpdateClusterEps(dval);
	}

	//analysis page
	//id text
	if (update_all || request.HasValue(gstCompIdColor))
	{
		fluo::Color color = glbin_comp_editor.GetColor();
		dlg->UpdateCompIdColor(color);
	}
	//size limiters
	if (update_all || request.HasValue(gstUseMin))
	{
		bval = glbin_comp_selector.GetUseMin();
		dlg->UpdateUseMin(bval);
	}
	if (update_all || request.HasValue(gstMinValue))
	{
		ival = glbin_comp_selector.GetMinNum();
		dlg->UpdateMinValue(ival);
	}
	if (update_all || request.HasValue(gstUseMax))
	{
		bval = glbin_comp_selector.GetUseMax();
		dlg->UpdateUseMax(bval);
	}
	if (update_all || request.HasValue(gstMaxValue))
	{
		ival = glbin_comp_selector.GetMaxNum();
		dlg->UpdateMaxValue(ival);
	}

	//analyzer settings
	if (update_all || request.HasValue(gstCompConsistent))
	{
		bval = glbin_comp_analyzer.GetConsistent();
		dlg->UpdateCompConsistent(bval);
	}
	if (update_all || request.HasValue(gstCompColocal))
	{
		bval = glbin_comp_analyzer.GetColocal();
		dlg->UpdateCompColocal(bval);
	}

	//output type
	if (update_all || request.HasValue(gstCompOutputType))
	{
		ival = glbin_comp_analyzer.GetColorType();
		dlg->UpdateCompOutputType(ival);
	}

	//Distances
	if (update_all || request.HasValue(gstDistNeighbor))
	{
		bval = glbin_comp_analyzer.GetUseDistNeighbor();
		dlg->UpdateDistNeighbor(bval);
	}
	if (update_all || request.HasValue(gstDistNeighborValue))
	{
		ival = glbin_comp_analyzer.GetDistNeighborNum();
		dlg->UpdateDistNeighborValue(ival);
	}
	if (update_all || request.HasValue(gstDistAllChan))
	{
		bval = glbin_comp_analyzer.GetUseDistAllchan();
		dlg->UpdateDistAllChan(bval);
	}

	//align center
	if (update_all || request.HasValue(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		dlg->UpdateAlignCenter(bval);
	}

	//output
	if (request.HasValue(gstCompGenOutput))
	{
		std::string str1, str2;
		str1 = ws2s(glbin_comp_generator.GetTitles());
		str2 = ws2s(glbin_comp_generator.GetValues());
		dlg->UpdateGrid(str1, str2);
	}

	if (request.HasValue(gstCompAnalysisResult))
	{
		size_t size = glbin_comp_analyzer.GetListSize();
		bool saved = false;
		if (size > m_max_lines)
		{
			ModalDlg fopendlg(dlg,
				wxString::Format("Component count is over %d. Save in a file?", m_max_lines),
				"", "", "Text file (*.txt)|*.txt",
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			int rval = fopendlg.ShowModal();
			if (rval == wxID_OK)
			{
				wxString filename = fopendlg.GetPath();
				std::wstring str = filename.ToStdWstring();
				glbin_comp_analyzer.OutputCompListFile(str, 1);
				saved = true;
			}
		}
		if (!saved)
		{
			std::string titles, values;
			glbin_comp_analyzer.OutputFormHeader(titles);
			glbin_comp_analyzer.OutputCompListStr(values, 0);
			dlg->UpdateGrid(titles, values);
		}
	}

	if (request.HasValue(gstCompListSelection))
		dlg->UpdateCompSelection();
}

void ComponentDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstCompGenerate))
		m_comp_gen_timer.start(100);
	if (request.HasValue(gstCompCluster))
		CompCluster();
	if (request.HasValue(gstCompAnalyze))
		CompAnalyze();
	if (request.HasValue(gstFixUpdate))
		FixUpdate();
	if (request.HasValue(gstCleanUpdate))
		CleanUpdate();
	if (request.HasValue(gstPlayCmd))
		PlayCmdUpdate();
	if (request.HasValue(gstResetCmd))
		ResetCmdUpdate();
	if (request.HasValue(gstLoadCmd))
		LoadCmdUpdate();
	if (request.HasValue(gstSaveCmd))
		SaveCmdUpdate();
	if (request.HasValue(gstCompFull))
		CompFull();
	if (request.HasValue(gstCompExclusive))
		CompExclusive();
	if (request.HasValue(gstCompAppend))
		CompAppend();
	if (request.HasValue(gstCompAll))
		CompAll();
	if (request.HasValue(gstCompClear))
		CompClear();
	if (request.HasValue(gstShuffle))
		Shuffle();
	if (request.HasValue(gstCompNew))
		CompNew();
	if (request.HasValue(gstCompAdd))
		CompAdd();
	if (request.HasValue(gstCompReplace))
		CompReplace();
	if (request.HasValue(gstCompClearBkg))
		CompCleanBkg();
	if (request.HasValue(gstCompCombine))
		CompCombine();
	if (request.HasValue(gstCompOutputMesh))
		CompOutputMesh();
	if (request.HasValue(gstCompOutputDist))
		OutputDistance();
}

ComponentDlg* ComponentDlgAgent::GetDialog() const
{
	return static_cast<ComponentDlg*>(GetWindow());
}

void ComponentDlgAgent::SetIter(int ival)
{
	glbin_comp_generator.SetIter(ival);
	UpdateDataToUI({ gstIteration, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetThresh(double dval)
{
	glbin_comp_generator.SetThresh(dval);
	UpdateDataToUI({ gstCompThreshold, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetUseDistField(bool bval)
{
	glbin_comp_generator.SetUseDistField(bval);
	UpdateDataToUI({ gstUseDistField, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetDistStrength(double dval)
{
	glbin_comp_generator.SetDistStrength(dval);
	UpdateDataToUI({ gstDistFieldStrength, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetDistFilterSize(int ival)
{
	glbin_comp_generator.SetDistFilterSize(ival);
	UpdateDataToUI({ gstDistFieldFilterSize, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetMaxDist(int ival)
{
	glbin_comp_generator.SetMaxDist(ival);
	UpdateDataToUI({ gstMaxDist, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetDistThresh(double dval)
{
	glbin_comp_generator.SetDistThresh(dval);
	UpdateDataToUI({ gstDistFieldThresh, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetUseDiff(bool bval)
{
	glbin_comp_generator.SetDiffusion(bval);
	UpdateDataToUI({ gstUseDiffusion, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetFalloff(double dval)
{
	glbin_comp_generator.SetFalloff(dval);
	UpdateDataToUI({ gstDiffusionFalloff, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetUseDensity(bool bval)
{
	glbin_comp_generator.SetDensity(bval);
	UpdateDataToUI({ gstUseDensityField, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetDensity(double dval)
{
	glbin_comp_generator.SetDensityThresh(dval);
	UpdateDataToUI({ gstDensityFieldThresh, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetVarth(double dval)
{
	glbin_comp_generator.SetVarThresh(dval);
	UpdateDataToUI({ gstDensityVarThresh, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetDensityWindowSize(int ival)
{
	glbin_comp_generator.SetDensityWinSize(ival);
	UpdateDataToUI({ gstDensityWindowSize, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetDensityStatsSize(int ival)
{
	glbin_comp_generator.SetDensityStatSize(ival);
	UpdateDataToUI({ gstDensityStatsSize, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetFixSize(int ival)
{
	glbin_comp_generator.SetFixSize(ival);
	UpdateDataToUI({ gstFixateSize, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetCleanIter(int ival)
{
	glbin_comp_generator.SetCleanIter(ival);
	UpdateDataToUI({ gstCleanIteration, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetCleanLimit(int ival)
{
	glbin_comp_generator.SetCleanSize(ival);
	UpdateDataToUI({ gstCleanSize, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetFixate(bool bval)
{
	glbin_comp_generator.SetFixate(bval);

	if (bval)
		glbin_comp_generator.Fixate();

	if (glbin_comp_generator.GetAutoCompGen())
	{
		bool clean = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(clean);
		NotifyViewUpdate({ gstFixateEnable });
	}
	else
		UpdateDataToUI({ gstFixateEnable });
}

void ComponentDlgAgent::SetClean(bool bval)
{
	glbin_comp_generator.SetClean(bval);
	UpdateDataToUI({ gstCleanEnable, gstRecordCmd });
	if (glbin_comp_generator.GetAutoCompGen())
		UpdateUIToData({ gstCompGenerate });
}

void ComponentDlgAgent::SetRecord(bool bval)
{
	glbin_comp_generator.SetRecordCmd(bval);
}

void ComponentDlgAgent::SetClusterMethod(int ival)
{
	glbin_clusterizer.SetMethod(ival);
	UpdateDataToUI({ gstClusterMethod });
}

void ComponentDlgAgent::SetClusterClnum(int ival)
{
	glbin_clusterizer.SetNum(ival);
}

void ComponentDlgAgent::SetClusterMaxiter(int ival)
{
	glbin_clusterizer.SetMaxIter(ival);
}

void ComponentDlgAgent::SetClusterTol(double dval)
{
	glbin_clusterizer.SetTol(static_cast<float>(dval));
}

void ComponentDlgAgent::SetClusterSize(int ival)
{
	glbin_clusterizer.SetSize(ival);
}

void ComponentDlgAgent::SetClustereps(double dval)
{
	glbin_clusterizer.SetEps(dval);
}

void ComponentDlgAgent::SetCompId(const std::string& str)
{
	glbin_comp_selector.SetId(str);
	unsigned long id = 0;
	if (TryToULong(str, id))
		glbin_comp_editor.SetId(id, false);
	else
		glbin_comp_editor.SetId(0, true);
	UpdateDataToUI({ gstCompIdColor });
}

void ComponentDlgAgent::SetUseMin(bool bval)
{
	glbin_comp_selector.SetUseMin(bval);
	glbin_comp_analyzer.SetUseMin(bval);
	UpdateDataToUI({ gstUseMin });
}

void ComponentDlgAgent::SetMin(int ival)
{
	glbin_comp_selector.SetMinNum(ival);
	glbin_comp_analyzer.SetMinNum(ival);
}

void ComponentDlgAgent::SetUseMax(bool bval)
{
	glbin_comp_selector.SetUseMax(bval);
	glbin_comp_analyzer.SetUseMax(bval);
	UpdateDataToUI({ gstUseMax });
}

void ComponentDlgAgent::SetMax(int ival)
{
	glbin_comp_selector.SetMaxNum(ival);
	glbin_comp_analyzer.SetMaxNum(ival);
}

void ComponentDlgAgent::SetConSize(int ival)
{
	glbin_comp_analyzer.SetSizeLimit(ival);
}

void ComponentDlgAgent::SetConsistent(bool bval)
{
	glbin_comp_analyzer.SetConsistent(bval);
}

void ComponentDlgAgent::SetColocal(bool bval)
{
	glbin_comp_analyzer.SetColocal(bval);
}

void ComponentDlgAgent::SetOutputType(int ival)
{
	glbin_comp_analyzer.SetChannelType(ival);
}

void ComponentDlgAgent::SetOutputChannels(int ival)
{
	glbin_comp_analyzer.SetColorType(ival);
	glbin_comp_analyzer.OutputChannels();
	NotifyViewUpdate({ gstListCtrl, gstTreeCtrl });
}

void ComponentDlgAgent::SetOutputAnnotData(int ival)
{
	glbin_comp_analyzer.SetAnnotType(ival);
	glbin_comp_analyzer.OutputAnnotData();
	NotifyViewUpdate({ gstListCtrl, gstTreeCtrl });
}

void ComponentDlgAgent::SetUseDistNeighbor(bool bval)
{
	glbin_comp_analyzer.SetUseDistNeighbor(bval);
	UpdateDataToUI({ gstDistNeighbor });
}

void ComponentDlgAgent::SetDistAllChan(bool bval)
{
	glbin_comp_analyzer.SetUseDistAllchan(bval);
}

void ComponentDlgAgent::SetDistNeighbor(int ival)
{
	glbin_comp_analyzer.SetDistNeighborNum(ival);
}

void ComponentDlgAgent::SetAlignCenter(bool bval)
{
	glbin_aligner.SetAlignCenter(bval);
	NotifyViewUpdate({ gstAlignCenter });
}

void ComponentDlgAgent::SetAlignPca(int ival)
{
	flrd::RulerList list;
	glbin_comp_analyzer.GetRulerListFromCelp(list);
	glbin_aligner.SetRulerList(list);
	glbin_aligner.SetAxisType(ival);
	glbin_aligner.SetView(glbin_current.render_view.lock());
	glbin_aligner.AlignPca(true);
	NotifyViewUpdate({ gstNull },
		{ glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock()) });
}

void ComponentDlgAgent::SetUseSel(bool bval)
{
	glbin_comp_generator.SetUseSel(bval);
	glbin_comp_analyzer.SetUseSel(bval);
}

void ComponentDlgAgent::SetUseMl(bool bval)
{
	glbin_comp_generator.SetUseMl(bval);
}

void ComponentDlgAgent::IncludeComps(const wxArrayInt& cols, const wxArrayInt& rows)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//get list of selected comps
	bool sel_all = cols.GetCount();
	flrd::CelpList cl;
	if (!sel_all)
	{
		std::vector<unsigned int> ids;
		std::vector<unsigned int> bids;
		int bn = glbin_comp_analyzer.GetBrickNum();
		dlg->AddSelArrayInt(ids, bids, rows, bn > 1);
		glbin_comp_analyzer.SetSelectedIds(ids, bids);
		glbin_comp_analyzer.GetSelectedCelp(cl, true);
	}
	else
		glbin_comp_analyzer.GetAllCelp(cl, true);

	glbin_comp_selector.SetList(cl);
	glbin_comp_selector.SelectList();

	NotifyViewUpdate({ gstCompAnalysisResult });
}

void ComponentDlgAgent::ExcludeComps(const wxArrayInt& cols, const wxArrayInt& rows)
{
	//get list of selected comps
	bool sel_all = cols.GetCount();
	flrd::CelpList cl;
	if (!sel_all)
	{
		std::vector<unsigned int> ids;
		std::vector<unsigned int> bids;
		//seli = m_output_grid->GetSelectedRows();
		int bn = glbin_comp_analyzer.GetBrickNum();
		AddSelArrayInt(ids, bids, rows, bn > 1);
		glbin_comp_analyzer.SetSelectedIds(ids, bids);
		glbin_comp_analyzer.GetSelectedCelp(cl, true);
	}
	else
		glbin_comp_analyzer.GetAllCelp(cl, true);

	glbin_comp_selector.SetList(cl);
	glbin_comp_selector.EraseList();

	NotifyViewUpdate({ gstCompAnalysisResult });
}

void ComponentDlgAgent::TimerGenerateComps()
{
	if (glbin_comp_generator.IsBusy())
	{
		m_comp_gen_timer.restart(100);
		return;
	}
	m_comp_gen_timer.stop();
	fluo::ValueCollection vc;
	//bool bval = m_use_sel_gen_chk->GetValue();
	//glbin_comp_generator.SetUseSel(bval);
	if (glbin_comp_generator.GetAutoThreshold())
		vc.insert({ gstBrushThreshold, gstCompThreshold, gstVolMeshThresh });
	glbin_comp_generator.GenerateComp();
	vc.insert({ gstCompGenOutput, gstMaskMode });
	NotifyViewUpdate(vc);
}

void ComponentDlgAgent::CompCluster()
{
	glbin_clusterizer.Compute();
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::CompAnalyze()
{
	glbin_comp_analyzer.Analyze();
	NotifyViewUpdate({ gstCompAnalysisResult });
}

void ComponentDlgAgent::FixUpdate()
{
	glbin_comp_generator.Fixate();

	if (glbin_comp_generator.GetAutoCompGen())
	{
		bool bval = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(bval);
		NotifyViewUpdate({ gstNull });
	}
}

void ComponentDlgAgent::CleanUpdate()
{
	glbin_comp_generator.Clean();
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::PlayCmdUpdate()
{
	glbin_comp_generator.PlayCmd(1.0);
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::ResetCmdUpdate()
{
	glbin_comp_generator.ResetCmd();
	NotifyViewUpdate({ gstRecordCmd });
}

void ComponentDlgAgent::LoadCmdUpdate()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Choose a FluoRender component generator macro command",
		"", "", "*.txt;*.dft", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	int rval = fopendlg.ShowModal();
	if (rval != wxID_OK)
		return;
	std::wstring filename = fopendlg.GetPath().ToStdWstring();

	glbin_comp_generator.LoadCmd(filename);
	UpdateDataToUI({ gstRecordCmd });
}

void ComponentDlgAgent::SaveCmdUpdate()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Save a FluoRender component generator macro command",
		"", "", "*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval != wxID_OK)
		return;

	std::wstring filename = fopendlg.GetPath().ToStdWstring();

	glbin_comp_generator.SaveCmd(filename);
}

void ComponentDlgAgent::CompFull()
{
	glbin_comp_selector.SelectFullComp();
	NotifyViewUpdate({ gstCompAnalysisResult, gstSelUndo });
}

void ComponentDlgAgent::CompExclusive()
{
	glbin_comp_selector.Exclusive();
	NotifyViewUpdate(
		{ gstCompAnalysisResult, gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate },
		{ glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock()) });
}

void ComponentDlgAgent::CompAppend()
{
	bool get_all = glbin_comp_selector.GetIdEmpty();
	glbin_comp_selector.Select(get_all);
	NotifyViewUpdate(
		{ gstCompAnalysisResult, gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate },
		{ glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock()) });
}

void ComponentDlgAgent::CompAll()
{
	glbin_comp_selector.All();
	NotifyViewUpdate(
		{ gstCompAnalysisResult, gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate },
		{ glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock()) });
}

void ComponentDlgAgent::CompClear()
{
	glbin_vol_selector.Clear();
	glbin_comp_selector.Clear();
	NotifyViewUpdate({ gstCompAnalysisResult, gstSelUndo });
}

void ComponentDlgAgent::Shuffle()
{
	//get current vd
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	vd->IncShuffle();
	NotifyViewUpdate({ gstCompAnalysisResult });
}

void ComponentDlgAgent::CompNew()
{
	glbin_comp_editor.NewId(false, false);
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::CompAdd()
{
	glbin_comp_editor.NewId(true, false);
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::CompReplace()
{
	glbin_comp_editor.ReplaceId();
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::CompCleanBkg()
{
	glbin_comp_editor.Clean(0);
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::CompCombine()
{
	glbin_comp_editor.CombineId();
	NotifyViewUpdate({ gstNull });
}

void ComponentDlgAgent::CompOutputMesh()
{
	auto vd = glbin_current.vol_data.lock();
	glbin_conv_vol_mesh.SetVolumeData(vd);
	glbin_conv_vol_mesh.Update(true);
	glbin_conv_vol_mesh.MergeVertices(true);
	auto md = glbin_conv_vol_mesh.GetMeshData();
	glbin_color_mesh.SetVolumeData(vd);
	glbin_color_mesh.SetMeshData(md);
	glbin_color_mesh.SetUseSel(true);
	glbin_color_mesh.SetUseComp(true);
	glbin_color_mesh.Update();
	auto view = glbin_current.render_view.lock();
	if (view && md &&
		glbin_data_manager.AddMeshData(md))
	{
		view->AddMeshData(md);
	}
	NotifyViewUpdate({ gstVolMeshThresh, gstVolMeshInfo, gstListCtrl, gstTreeCtrl },
		{ glbin_coordinator.FindRenderCanvasAgent(view) });
}

void ComponentDlgAgent::OutputDistance()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		std::string str = filename.ToStdString();
		std::ofstream outfile;
		outfile.open(str, std::ofstream::out);
		//output result matrix
		glbin_comp_analyzer.OutputDistance(outfile);
		outfile.close();
	}
}

