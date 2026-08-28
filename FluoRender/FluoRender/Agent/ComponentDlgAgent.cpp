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

ComponentDlgAgent::ComponentDlgAgent(
	ComponentDlg* dlg) :
	Agent(dlg)
{

}

bool ComponentDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void ComponentDlgAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void ComponentDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	int ival;
	double dval;
	bool bval;

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		dlg->UpdateUseSelection(glbin_comp_generator.GetUseSel(),
			glbin_comp_analyzer.GetUseSel());
	}

	if (update_all || FOUND_VALUE(gstUseMachineLearning))
	{
		dlg->UpdateUseMachineLearning(glbin_comp_generator.GetUseMl());
	}

	bool brush_update = FOUND_VALUE(gstBrushCountAutoUpdate);
	if (FOUND_VALUE(gstCompAutoUpdate) ||
		brush_update)
	{
		auto mode = glbin_vol_selector.GetSelectMode();
		if (mode == flrd::SelectMode::Segment ||
			mode == flrd::SelectMode::Mesh)
			return;
		if (brush_update)
		{
			if (glbin_comp_generator.GetUseSel())
			{
				auto vd = glbin_comp_generator.GetVolumeData();
				if (!vd->GetLabel(false))
					return;
			}
			else
				return;
		}
		if (glbin_comp_generator.GetAutoCompGen())
			dlg->LaunchAutoUpdateTimer();
	}

	//comp generate page
	if (update_all || FOUND_VALUE(gstIteration))
	{
		ival = glbin_comp_generator.GetIter();
		dlg->UpdateIteration(ival);
	}
	if (update_all || FOUND_VALUE(gstCompThreshold))
	{
		dval = glbin_comp_generator.GetThresh();
		dlg->UpdateCompThreshold(dval);
	}
	//diffusion
	if (update_all || FOUND_VALUE(gstUseDiffusion))
	{
		bval = glbin_comp_generator.GetDiffusion();
		dlg->UpdateDiffusion(bval);
	}
	if (update_all || FOUND_VALUE(gstDiffusionFalloff))
	{
		dval = glbin_comp_generator.GetFalloff();
		dlg->UpdateDiffusionFalloff(dval);
	}
	//density
	if (update_all || FOUND_VALUE(gstUseDensityField))
	{
		bval = glbin_comp_generator.GetDensity();
		dlg->UpdateUseDensityField(bval);
	}
	if (update_all || FOUND_VALUE(gstDensityFieldThresh))
	{
		dval = glbin_comp_generator.GetDensityThresh();
		dlg->UpdateDensityFieldThresh(dval);
	}
	if (update_all || FOUND_VALUE(gstDensityVarThresh))
	{
		dval = glbin_comp_generator.GetVarThresh();
		dlg->UpdateDensityVarThresh(dval);
	}
	if (update_all || FOUND_VALUE(gstDensityWindowSize))
	{
		ival = glbin_comp_generator.GetDensityWinSize();
		dlg->UpdateDensityWindowSize(ival);
	}
	if (update_all || FOUND_VALUE(gstDensityStatsSize))
	{
		ival = glbin_comp_generator.GetDensityStatSize();
		dlg->UpdateDensityStatsSize(ival);
	}
	//dist
	if (update_all || FOUND_VALUE(gstUseDistField))
	{
		bval = glbin_comp_generator.GetUseDistField();
		dlg->UpdateUseDistField(bval);
	}
	if (update_all || FOUND_VALUE(gstDistFieldStrength))
	{
		dval = glbin_comp_generator.GetDistStrength();
		dlg->UpdateDistFieldStrength(dval);
	}
	if (update_all || FOUND_VALUE(gstDistFieldFilterSize))
	{
		ival = glbin_comp_generator.GetDistFilterSize();
		dlg->UpdateDistFieldFilterSize(ival);
	}
	if (update_all || FOUND_VALUE(gstMaxDist))
	{
		ival = glbin_comp_generator.GetMaxDist();
		dlg->UpdateMaxDist(ival);
	}
	if (update_all || FOUND_VALUE(gstDistFieldThresh))
	{
		dval = glbin_comp_generator.GetDistThresh();
		dlg->UpdateDistFieldThresh(dval);
	}

	//fixate
	if (update_all || FOUND_VALUE(gstFixateEnable))
	{
		bval = glbin_comp_generator.GetFixate();
		dlg->UpdateFixateEnable(bval);
	}
	if (update_all || FOUND_VALUE(gstGrowFixed))
	{
		bval = glbin_comp_generator.GetGrowFixed();
		dlg->UpdateGrowFixed(bval);
	}
	if (update_all || FOUND_VALUE(gstFixateSize))
	{
		ival = glbin_comp_generator.GetFixSize();
		dlg->UpdateFixateSize(ival);
	}
	//clean
	if (update_all || FOUND_VALUE(gstCleanEnable))
	{
		bval = glbin_comp_generator.GetClean();
		dlg->UpdateCleanEnable(bval);
	}
	if (update_all || FOUND_VALUE(gstCleanIteration))
	{
		ival = glbin_comp_generator.GetCleanIter();
		dlg->UpdateCleanIteration(ival);
	}
	if (update_all || FOUND_VALUE(gstCleanSize))
	{
		ival = glbin_comp_generator.GetCleanSize();
		dlg->UpdateCleanSize(ival);
	}
	//record
	if (update_all || FOUND_VALUE(gstRecordCmd))
	{
		ival = glbin_comp_generator.GetCmdNum();
		bval = glbin_comp_generator.GetRecordCmd();
		dlg->UpdateRecordCmd(ival, bval);
	}

	//cluster page
	if (update_all || FOUND_VALUE(gstClusterMethod))
	{
		ival = glbin_clusterizer.GetMethod();
		dlg->UpdateClusterMethod(ival);
	}
	//parameters
	if (update_all || FOUND_VALUE(gstClusterNum))
	{
		ival = glbin_clusterizer.GetNum();
		dlg->UpdateClusterNum(ival);
	}
	if (update_all || FOUND_VALUE(gstClusterMaxIter))
	{
		ival = glbin_clusterizer.GetMaxIter();
		dlg->UpdateClusterMaxIter(ival);
	}
	if (update_all || FOUND_VALUE(gstClusterTol))
	{
		dval = glbin_clusterizer.GetTol();
		dlg->UpdateClusterTol(dval);
	}
	if (update_all || FOUND_VALUE(gstClusterSize))
	{
		ival = glbin_clusterizer.GetSize();
		dlg->UpdateClusterSize(ival);
	}
	if (update_all || FOUND_VALUE(gstClusterEps))
	{
		dval = glbin_clusterizer.GetEps();
		dlg->UpdateClusterEps(dval);
	}

	//analysis page
	//id text
	if (update_all || FOUND_VALUE(gstCompIdColor))
	{
		fluo::Color color = glbin_comp_editor.GetColor();
		dlg->UpdateCompIdColor(color);
	}
	//size limiters
	if (update_all || FOUND_VALUE(gstUseMin))
	{
		bval = glbin_comp_selector.GetUseMin();
		dlg->UpdateUseMin(bval);
	}
	if (update_all || FOUND_VALUE(gstMinValue))
	{
		ival = glbin_comp_selector.GetMinNum();
		dlg->UpdateMinValue(ival);
	}
	if (update_all || FOUND_VALUE(gstUseMax))
	{
		bval = glbin_comp_selector.GetUseMax();
		dlg->UpdateUseMax(bval);
	}
	if (update_all || FOUND_VALUE(gstMaxValue))
	{
		ival = glbin_comp_selector.GetMaxNum();
		dlg->UpdateMaxValue(ival);
	}

	//analyzer settings
	if (update_all || FOUND_VALUE(gstCompConsistent))
	{
		bval = glbin_comp_analyzer.GetConsistent();
		m_consistent_check->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(gstCompColocal))
	{
		bval = glbin_comp_analyzer.GetColocal();
		m_colocal_check->SetValue(bval);
	}

	//output type
	if (update_all || FOUND_VALUE(gstCompOutputType))
	{
		ival = glbin_comp_analyzer.GetColorType();
		m_output_multi_rb->SetValue(ival == 1);
		m_output_rgb_rb->SetValue(ival == 2);
	}

	//Distances
	if (update_all || FOUND_VALUE(gstDistNeighbor))
	{
		bval = glbin_comp_analyzer.GetUseDistNeighbor();
		m_dist_neighbor_check->SetValue(bval);
		m_dist_neighbor_sldr->Enable(bval);
		m_dist_neighbor_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDistNeighborValue))
	{
		ival = glbin_comp_analyzer.GetDistNeighborNum();
		m_dist_neighbor_sldr->ChangeValue(ival);
		m_dist_neighbor_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstDistAllChan))
	{
		bval = glbin_comp_analyzer.GetUseDistAllchan();
		m_dist_all_chan_check->SetValue(bval);
	}

	//align center
	if (update_all || FOUND_VALUE(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		m_align_center_chk->SetValue(bval);
	}

	//output
	if (FOUND_VALUE(gstCompGenOutput))
	{
		DeleteGridRows();
		wxString str1, str2;
		str1 = glbin_comp_generator.GetTitles();
		str2 = glbin_comp_generator.GetValues();
		OutputAnalysis(str1, str2);
	}

	if (FOUND_VALUE(gstCompAnalysisResult))
	{
		DeleteGridRows();
		size_t size = glbin_comp_analyzer.GetListSize();
		bool saved = false;
		if (size > m_max_lines)
		{
			ModalDlg fopendlg(this,
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
			wxString str1(titles), str2(values);
			OutputAnalysis(str1, str2);
		}
	}

	if (FOUND_VALUE(gstCompListSelection))
		UpdateCompSelection();
}

void ComponentDlgAgent::UpdateData(const UpdateRequest& request)
{

}

ComponentDlg* ComponentDlgAgent::GetDialog() const
{
	return static_cast<ComponentDlg*>(GetWindow());
}
