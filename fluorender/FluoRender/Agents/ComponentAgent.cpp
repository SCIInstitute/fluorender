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

#include <ComponentAgent.hpp>
#include <ComponentDlg.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <AnnotationFactory.hpp>
#include <BrushToolAgent.hpp>
#include <ColocalAgent.hpp>
#include <VolumeData.hpp>
#include <CompAnalyzer.h>
#include <CompSelector.h>
#include <CompEditor.h>
#include <Ruler.h>
#include <RulerAlign.h>
#include <ClusterMethod.h>
#include <kmeans.h>
#include <exmax.h>
#include <dbscan.h>
#include <VolumeRenderer.h>
#include <cctype>
#include <fstream>

using namespace fluo;

ComponentAgent::ComponentAgent(ComponentDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void ComponentAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* ComponentAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void ComponentAgent::UpdateAllSettings()
{
	bool bval;
	int ival;
	long lval;
	double dval;
	//update ui
	getValue(gstUseSelection, bval);
	dlg_.m_use_sel_chk->SetValue(bval);
	//comp generate page
	getValue(gstIteration, lval);
	dlg_.m_iter_text->SetValue(wxString::Format("%d", lval));
	getValue(gstThreshold, dval);
	dlg_.m_thresh_text->SetValue(wxString::Format("%.3f", dval));
	//dist
	getValue(gstUseDistField, bval);
	dlg_.m_use_dist_field_check->SetValue(bval);
	getValue(gstDistFieldStrength, dval);
	dlg_.m_dist_strength_text->SetValue(wxString::Format("%.3f", dval));
	getValue(gstDistFieldFilterSize, lval);
	dlg_.m_dist_filter_size_text->SetValue(wxString::Format("%d", lval));
	getValue(gstMaxDist, lval);
	dlg_.m_max_dist_text->SetValue(wxString::Format("%d", lval));
	getValue(gstDistFieldThresh, dval);
	dlg_.m_dist_thresh_text->SetValue(wxString::Format("%.3f", dval));
	//diffusion
	getValue(gstUseDiffusion, bval);
	dlg_.m_diff_check->SetValue(bval);
	getValue(gstDiffusionFalloff, dval);
	dlg_.m_falloff_text->SetValue(wxString::Format("%.3f", dval));
	//density
	getValue(gstUseDensityField, bval);
	dlg_.m_density_check->SetValue(bval);
	getValue(gstDensityFieldThresh, dval);
	dlg_.m_density_text->SetValue(wxString::Format("%.3f", dval));
	getValue(gstDensityVarThresh, dval);
	dlg_.m_varth_text->SetValue(wxString::Format("%.4f", dval));
	getValue(gstDensityWindowSize, lval);
	dlg_.m_density_window_size_text->SetValue(wxString::Format("%d", lval));
	getValue(gstDensityStatsSize, lval);
	dlg_.m_density_stats_size_text->SetValue(wxString::Format("%d", lval));
	//fixate
	getValue(gstFixateEnable, bval);
	dlg_.m_fixate_check->SetValue(bval);
	getValue(gstFixateSize, lval);
	dlg_.m_fix_size_text->SetValue(wxString::Format("%d", lval));
	//clean
	getValue(gstCleanEnable, bval);
	dlg_.m_clean_check->SetValue(bval);
	getValue(gstCleanIteration, lval);
	dlg_.m_clean_iter_text->SetValue(wxString::Format("%d", lval));
	getValue(gstCleanSize, lval);
	dlg_.m_clean_limit_text->SetValue(wxString::Format("%d", lval));
	//record
	ival = m_command.size();
	dlg_.m_cmd_count_text->SetValue(wxString::Format("%d", ival));
	//cluster page
	getValue(gstClusterMethod, lval);
	dlg_.m_cluster_method_kmeans_rd->SetValue(lval==0);
	dlg_.m_cluster_method_exmax_rd->SetValue(lval==1);
	dlg_.m_cluster_method_dbscan_rd->SetValue(lval==2);
	//parameters
	getValue(gstClusterNum, lval);
	dlg_.m_cluster_clnum_text->SetValue(wxString::Format("%d", lval));
	getValue(gstClusterMaxIter, lval);
	dlg_.m_cluster_maxiter_text->SetValue(wxString::Format("%d", lval));
	getValue(gstClusterTol, dval);
	dlg_.m_cluster_tol_text->SetValue(wxString::Format("%.2f", dval));
	getValue(gstClusterSize, lval);
	dlg_.m_cluster_size_text->SetValue(wxString::Format("%d", lval));
	getValue(gstClusterEps, dval);
	dlg_.m_cluster_eps_text->SetValue(wxString::Format("%.1f", dval));
	//selection
	getValue(gstUseMin, bval);
	dlg_.m_analysis_min_check->SetValue(bval);
	dlg_.m_analysis_min_spin->Enable(bval);
	getValue(gstMinValue, lval);
	dlg_.m_analysis_min_spin->SetValue(lval);
	getValue(gstUseMax, bval);
	dlg_.m_analysis_max_check->SetValue(bval);
	dlg_.m_analysis_max_spin->Enable(bval);
	getValue(gstMaxValue, lval);
	dlg_.m_analysis_max_spin->SetValue(lval);
	//options
	getValue(gstCompConsistent, bval);
	dlg_.m_consistent_check->SetValue(bval);
	getValue(gstCompColocal, bval);
	dlg_.m_colocal_check->SetValue(bval);
	//output type
	getValue(gstCompOutputType, lval);
	dlg_.m_output_multi_rb->SetValue(lval==1);
	dlg_.m_output_rgb_rb->SetValue(lval==2);
	//neighbors
	getValue(gstDistNeighbor, bval);
	dlg_.m_dist_neighbor_check->SetValue(bval);
	dlg_.m_dist_neighbor_sldr->Enable(bval);
	dlg_.m_dist_neighbor_text->Enable(bval);
	getValue(gstDistNeighborValue, lval);
	dlg_.m_dist_neighbor_sldr->SetValue(lval);
	getValue(gstDistAllChan, bval);
	dlg_.m_dist_all_chan_check->SetValue(bval);
	//output
	getValue(gstHoldHistory, bval);
	dlg_.m_history_chk->SetValue(bval);
}

void ComponentAgent::LoadSettings(const wxString &filename)
{
	bool get_basic = false;
	wxString str;
	if (!wxFileExists(filename))
	{
		wxString expath = glbin.getExecutablePath();
		expath = wxPathOnly(expath);
		str = expath + GETSLASH() + "default_component_settings.dft";
		get_basic = true;
	}
	else
		str = filename;
	wxFileInputStream is(str);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	bool bval;
	int ival;
	long lval;
	double dval;
	//basic settings
	fconfig.Read("use_sel", &bval); setValue(gstUseSelection, bval);
	fconfig.Read("iter", &lval); setValue(gstIteration, lval);
	fconfig.Read("thresh", &dval); setValue(gstThreshold, dval);
	fconfig.Read("use_dist_field", &bval); setValue(gstUseDistField, bval);
	fconfig.Read("dist_strength", &dval); setValue(gstDistFieldStrength, dval);
	fconfig.Read("dist_filter_size", &lval); setValue(gstDistFieldFilterSize, lval);
	fconfig.Read("max_dist", &lval); setValue(gstMaxDist, lval);
	fconfig.Read("dist_thresh", &dval); setValue(gstDistFieldThresh, dval);
	fconfig.Read("diff", &bval); setValue(gstUseDiffusion, bval);
	fconfig.Read("falloff", &dval); setValue(gstDiffusionFalloff, dval);
	fconfig.Read("density", &bval); setValue(gstUseDensityField, bval);
	fconfig.Read("density_thresh", &dval); setValue(gstDensityFieldThresh, dval);
	fconfig.Read("varth", &dval); setValue(gstDensityVarThresh, dval);
	fconfig.Read("density_window_size", &lval); setValue(gstDensityWindowSize, lval);
	fconfig.Read("density_stats_size", &lval); setValue(gstDensityStatsSize, lval);
	fconfig.Read("clean", &bval); setValue(gstCleanEnable, bval);
	fconfig.Read("clean_iter", &lval); setValue(gstCleanIteration, lval);
	fconfig.Read("clean_size_vl", &lval); setValue(gstCleanSize, lval);
	//cluster
	fconfig.Read("cluster_method", &lval); setValue(gstClusterMethod, lval);
	//parameters
	fconfig.Read("cluster_clnum", &lval); setValue(gstClusterNum, lval);
	fconfig.Read("cluster_maxiter", &lval); setValue(gstClusterMaxIter, lval);
	fconfig.Read("cluster_tol", &dval); setValue(gstClusterTol, dval);
	fconfig.Read("cluster_size", &lval); setValue(gstClusterSize, lval);
	fconfig.Read("cluster_eps", &dval); setValue(gstClusterEps, dval);
	//selection
	fconfig.Read("use_min", &bval); setValue(gstUseMin, bval);
	fconfig.Read("min_num", &lval); setValue(gstMinValue, lval);
	fconfig.Read("use_max", &bval); setValue(gstUseMax, bval);
	fconfig.Read("max_num", &lval); setValue(gstMaxValue, lval);
	//colocalization
	fconfig.Read("colocal", &bval); setValue(gstCompColocal, bval);
	//output
	fconfig.Read("output_type", &lval); setValue(gstCompOutputType, lval);
}

void ComponentAgent::SaveSettings(const wxString &filename)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_component_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	bool bval;
	int ival;
	long lval;
	double dval;
	//comp generate settings
	getValue(gstUseSelection, bval); fconfig.Write("use_sel", bval);
	getValue(gstIteration, lval); fconfig.Write("iter", lval);
	getValue(gstThreshold, dval); fconfig.Write("thresh", dval);
	getValue(gstUseDistField, bval); fconfig.Write("use_dist_field", bval);
	getValue(gstDistFieldStrength, dval); fconfig.Write("dist_strength", dval);
	getValue(gstDistFieldFilterSize, lval); fconfig.Write("dist_filter_size", lval);
	getValue(gstMaxDist, lval); fconfig.Write("max_dist", lval);
	getValue(gstDistFieldThresh, dval); fconfig.Write("dist_thresh", dval);
	getValue(gstUseDiffusion, bval); fconfig.Write("diff", bval);
	getValue(gstDiffusionFalloff, dval); fconfig.Write("falloff", dval);
	getValue(gstUseDensityField, bval); fconfig.Write("density", bval);
	getValue(gstDensityFieldThresh, dval); fconfig.Write("density_thresh", dval);
	getValue(gstDensityVarThresh, dval); fconfig.Write("varth", dval);
	getValue(gstDensityWindowSize, lval); fconfig.Write("density_window_size", lval);
	getValue(gstDensityStatsSize, lval); fconfig.Write("density_stats_size", lval);
	getValue(gstCleanEnable, bval); fconfig.Write("clean", bval);
	getValue(gstCleanIteration, lval); fconfig.Write("clean_iter", lval);
	getValue(gstCleanSize, lval); fconfig.Write("clean_size_vl", lval);
	//cluster
	getValue(gstClusterMethod, lval); fconfig.Write("cluster_method", lval);
	//parameters
	getValue(gstClusterNum, lval); fconfig.Write("cluster_clnum", lval);
	getValue(gstClusterMaxIter, lval); fconfig.Write("cluster_maxiter", lval);
	getValue(gstClusterTol, dval); fconfig.Write("cluster_tol", dval);
	getValue(gstClusterSize, lval); fconfig.Write("cluster_size", lval);
	getValue(gstClusterEps, dval); fconfig.Write("cluster_eps", dval);
	//selection
	getValue(gstUseMin, bval); fconfig.Write("use_min", bval);
	getValue(gstMinValue, lval); fconfig.Write("min_num", lval);
	getValue(gstUseMax, bval); fconfig.Write("use_max", bval);
	getValue(gstMaxValue, lval); fconfig.Write("max_num", lval);
	//colocalization
	getValue(gstCompColocal, bval); fconfig.Write("colocal", bval);
	//output
	getValue(gstCompOutputType, lval); fconfig.Write("output_type", lval);

	wxString str;
	if (filename == "")
	{
		wxString expath = glbin.getExecutablePath();
		expath = wxPathOnly(expath);
		str = expath + GETSLASH() + "default_component_settings.dft";
	}
	else
		str = filename;
	SaveConfig(fconfig, str);
}

void ComponentAgent::Analyze()
{
	bool bval;
	getValue(gstUseSelection, bval);
	Analyze(bval);
}

void ComponentAgent::Analyze(bool sel)
{
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!analyzer) return;
	analyzer->SetVolume(vd);
	int bn = vd->GetAllBrickNum();
	//m_prog_bit = 97.0f / float(bn * 2 + (m_consistent ? 1 : 0));
	//m_prog = 0.0f;

	//boost::signals2::connection connection =
	//	m_comp_analyzer.m_sig_progress.connect(std::bind(
	//	&ComponentDlg::UpdateProgress, this));

	bool colocal, consistent;
	getValue(gstCompColocal, colocal);
	getValue(gstCompConsistent, consistent);
	if (colocal)
	{
		analyzer->ClearCoVolumes();
		VolumeList list = view->GetVolList();
		for (auto vdi : list)
		{
			if (vdi != vd)
				analyzer->AddCoVolume(vdi);
		}
	}
	analyzer->Analyze(sel, consistent, colocal);

	if (consistent)
	{
		//invalidate label mask in gpu
		vd->GetRenderer()->clear_tex_label();
		//m_view->Update(39);
	}

	if (analyzer->GetListSize() < 10000)
	//{
	//	wxFileDialog *fopendlg = new wxFileDialog(
	//		this, "Save Analysis Data", "", "",
	//		"Text file (*.txt)|*.txt",
	//		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	//	int rval = fopendlg->ShowModal();
	//	if (rval == wxID_OK)
	//	{
	//		wxString filename = fopendlg->GetPath();
	//		string str = filename.ToStdString();
	//		m_comp_analyzer.OutputCompListFile(str, 1);
	//	}
	//	if (fopendlg)
	//		delete fopendlg;
	//}
	//else
	{
		string titles, values;
		analyzer->OutputFormHeader(titles);
		analyzer->OutputCompListStr(values, 0);
		dlg_.SetOutput(titles, values);
	}

	//connection.disconnect();
}

void ComponentAgent::GenerateComp()
{
	bool use_sel, run_cmd;
	getValue(gstUseSelection, use_sel);
	getValue(gstRunCmd, run_cmd);
	GenerateComp(use_sel, run_cmd);
}

void ComponentAgent::GenerateComp(bool use_sel, bool run_cmd)
{
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	bool rec_cmd;
	getValue(gstRecordCmd, rec_cmd);

	bool clean; long clean_iter, clean_size;
	getValue(gstCleanEnable, clean);
	getValue(gstCleanIteration, clean_iter);
	getValue(gstCleanSize, clean_size);
	if (!clean)
	{
		clean_iter = 0;
		clean_size = 0;
	}

	//get brick number
	int bn = vd->GetAllBrickNum();
	double scale;
	vd->getValue(gstIntScale, scale);

	std::string titles, values;
	flrd::ComponentGenerator cg(vd);
	//boost::signals2::connection preconn =
	//	cg.prework.connect(std::bind(
	//		&ComponentAgent::StartTimer, this, std::placeholders::_1));
	//boost::signals2::connection postconn =
	//	cg.postwork.connect(std::bind(
	//		&ComponentAgent::StopTimer, this, std::placeholders::_1));
	m_tps.push_back(std::chrono::high_resolution_clock::now());

	cg.SetUseMask(use_sel);

	vd->AddEmptyMask(cg.GetUseMask() ? 2 : 1, true);//select all if no mask, otherwise keep
	bool fixate;
	getValue(gstFixateEnable, fixate);
	if (fixate && vd->GetLabel(false))
	{
		vd->LoadLabel2();
		long lval;
		getValue(gstFixateSize, lval);
		cg.SetIDBit(lval);
	}
	else
	{
		vd->AddEmptyLabel(0, !use_sel);
		cg.ShuffleID();
	}

	bool use_dist_field, use_density_field;
	getValue(gstUseDistField, use_dist_field);
	getValue(gstUseDensityField, use_density_field);
	if (use_dist_field)
	{
		if (use_density_field)
		{
			bool use_diff;
			long iteration, dist_filter_size, max_dist, density_window_size, density_stat_size;
			double threshold, thresh_scale, falloff, dist_thresh, dist_strength, density_thresh, density_var;
			getValue(gstUseDiffusion, use_diff);
			getValue(gstIteration, iteration);
			getValue(gstThreshold, threshold);
			getValue(gstThreshScale, thresh_scale);
			getValue(gstDiffusionFalloff, falloff);
			getValue(gstDistFieldFilterSize, dist_filter_size);
			getValue(gstMaxDist, max_dist);
			getValue(gstDistFieldThresh, dist_thresh);
			getValue(gstDistFieldStrength, dist_strength);
			getValue(gstDensityWindowSize, density_window_size);
			getValue(gstDensityStatsSize, density_stat_size);
			getValue(gstDensityFieldThresh, density_thresh);
			getValue(gstDensityVarThresh, density_var);
			cg.DistDensityField(
				use_diff,
				iteration,
				threshold*thresh_scale,
				falloff,
				dist_filter_size,
				max_dist,
				dist_thresh,
				dist_strength,
				density_window_size,
				density_stat_size,
				density_thresh,
				density_var,
				scale);
		}
		else
		{
			bool use_diff;
			long iteration, dist_filter_size, max_dist;
			double threshold, thresh_scale, falloff, dist_thresh, dist_strength;
			getValue(gstUseDiffusion, use_diff);
			getValue(gstIteration, iteration);
			getValue(gstThreshold, threshold);
			getValue(gstThreshScale, thresh_scale);
			getValue(gstDiffusionFalloff, falloff);
			getValue(gstDistFieldFilterSize, dist_filter_size);
			getValue(gstMaxDist, max_dist);
			getValue(gstDistFieldThresh, dist_thresh);
			getValue(gstDistFieldStrength, dist_strength);
			cg.DistGrow(
				use_diff,
				iteration,
				threshold*thresh_scale,
				falloff,
				dist_filter_size,
				max_dist,
				dist_thresh,
				scale,
				dist_strength);
		}
	}
	else
	{
		if (use_density_field)
		{
			bool use_diff;
			long iteration, density_window_size, density_stat_size;
			double threshold, thresh_scale, falloff, density_thresh, density_var;
			getValue(gstUseDiffusion, use_diff);
			getValue(gstIteration, iteration);
			getValue(gstThreshold, threshold);
			getValue(gstThreshScale, thresh_scale);
			getValue(gstDiffusionFalloff, falloff);
			getValue(gstDensityWindowSize, density_window_size);
			getValue(gstDensityStatsSize, density_stat_size);
			getValue(gstDensityFieldThresh, density_thresh);
			getValue(gstDensityVarThresh, density_var);
			cg.DensityField(
				density_window_size,
				density_stat_size,
				use_diff,
				iteration,
				threshold*thresh_scale,
				falloff,
				density_thresh,
				density_var,
				scale);
		}
		else
		{
			bool use_diff;
			long iteration;
			double threshold, thresh_scale, falloff;
			getValue(gstUseDiffusion, use_diff);
			getValue(gstIteration, iteration);
			getValue(gstThreshold, threshold);
			getValue(gstThreshScale, thresh_scale);
			getValue(gstDiffusionFalloff, falloff);
			cg.Grow(
				use_diff,
				iteration,
				threshold*thresh_scale,
				falloff,
				scale);
		}
	}

	if (clean_iter > 0)
		cg.Cleanup(clean_iter, clean_size);

	if (bn > 1)
		cg.FillBorders(0.1);

	m_tps.push_back(std::chrono::high_resolution_clock::now());
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(
			m_tps.back() - m_tps.front());
	bool bval;
	getValue(gstTestSpeed, bval);
	if (bval)
	{
		titles += "Function\t";
		titles += "Time\n";
		values += "Total\t";
	}
	else
	{
		titles += "Total time\n";
	}
	values += wxString::Format("%.4f", time_span.count());
	values += " sec.\n";
	dlg_.SetOutput(titles, values);

	//update
	//m_view->Update(39);

	if (run_cmd && rec_cmd)
		AddCmd("generate");
}

void ComponentAgent::Fixate(bool command)
{
	Renderview* view = getObject();
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd)
		return;
	vd->PushLabel(true);

	bool bval;
	getValue(gstRecordCmd, bval);
	if (command && bval)
		AddCmd("fixate");

	saveValue(gstCleanEnable);
	Event event;
	OnAutoUpdate(event);
	drawValue(gstCleanEnable);
}

void ComponentAgent::Clean()
{
	bool use_sel, run_command;
	getValue(gstUseSelection, use_sel);
	getValue(gstRunCmd, run_command);
	Clean(use_sel, run_command);
}

void ComponentAgent::Clean(bool use_sel, bool command)
{
	VolumeData* vd = getObject()->GetCurrentVolume();
	if (!vd) return;

	long clean_iter, clean_size;
	getValue(gstCleanIteration, clean_iter);
	getValue(gstCleanSize, clean_size);

	//get brick number
	int bn = vd->GetAllBrickNum();

	flrd::ComponentGenerator cg(vd);
	cg.SetUseMask(use_sel);
	vd->AddEmptyMask(1, !use_sel);
	if (bn > 1)
		cg.ClearBorders();
	if (clean_iter > 0)
		cg.Cleanup(clean_iter, clean_size);
	if (bn > 1)
		cg.FillBorders(0.1);

	//m_view->Update(39);
	bool bval;
	getValue(gstRecordCmd, bval);
	if (command && bval)
		AddCmd("clean");
}

void ComponentAgent::CompFull()
{
	bool bval;
	//get id
	std::string str;
	getValue(gstCompIdStr, str);
	if (str.empty())
	{
		//get current mask
		VolumeData* vd = getObject()->GetCurrentVolume();
		flrd::ComponentSelector comp_selector(vd);
		//cell size filter
		long lval;
		getValue(gstUseMin, bval);
		getValue(gstMinValue, lval);
		comp_selector.SetMinNum(bval, lval);
		getValue(gstUseMax, bval);
		getValue(gstMaxValue, lval);
		comp_selector.SetMaxNum(bval, lval);
		flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
		comp_selector.SetAnalyzer(analyzer);
		comp_selector.CompFull();
	}
	else
	{
		CompAppend();
	}

	//m_view->Update(39);

	getObject()->getValue(gstPaintCount, bval);
	if (bval) glbin_agtf->getBrushToolAgent()->Update(0);
	glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
	getObject()->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->getColocalAgent()->Run();
}

void ComponentAgent::CompAppend()
{
	std::string str;
	getValue(gstCompIdStr, str);
	//get id
	unsigned int id;
	int brick_id;
	bool get_all = GetIds(str, id, brick_id);
	get_all = !get_all;

	//get current mask
	VolumeData* vd = getObject()->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.SetId(flrd::Cell::GetKey(id, brick_id));

	//cell size filter
	bool bval;
	long lval;
	getValue(gstUseMin, bval);
	getValue(gstMinValue, lval);
	comp_selector.SetMinNum(bval, lval);
	getValue(gstUseMax, bval);
	getValue(gstMaxValue, lval);
	comp_selector.SetMaxNum(bval, lval);
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	comp_selector.SetAnalyzer(analyzer);
	comp_selector.Select(get_all);

	//m_view->Update(39);

	getObject()->getValue(gstPaintCount, bval);
	if (bval) glbin_agtf->getBrushToolAgent()->Update(0);
	glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
	getObject()->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->getColocalAgent()->Run();
}

void ComponentAgent::CompExclusive()
{
	bool bval;
	std::string str;
	getValue(gstCompIdStr, str);
	//get id
	unsigned int id;
	int brick_id;

	if (!GetIds(str, id, brick_id))
		return;
	//get current mask
	VolumeData* vd = getObject()->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.SetId(flrd::Cell::GetKey(id, brick_id));

	//cell size filter
	long lval;
	getValue(gstUseMin, bval);
	getValue(gstMinValue, lval);
	comp_selector.SetMinNum(bval, lval);
	getValue(gstUseMax, bval);
	getValue(gstMaxValue, lval);
	comp_selector.SetMaxNum(bval, lval);
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	comp_selector.SetAnalyzer(analyzer);
	comp_selector.Exclusive();

	//m_view->Update(39);

	getObject()->getValue(gstPaintCount, bval);
	if (bval) glbin_agtf->getBrushToolAgent()->Update(0);
	glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
	getObject()->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->getColocalAgent()->Run();
}

void ComponentAgent::CompAll()
{
	//get current vd
	VolumeData* vd = getObject()->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.All();

	//m_view->Update(39);

	bool bval;
	getObject()->getValue(gstPaintCount, bval);
	if (bval) glbin_agtf->getBrushToolAgent()->Update(0);
	glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
	getObject()->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->getColocalAgent()->Run();
}

void ComponentAgent::CompClear()
{
	//get current vd
	VolumeData* vd = getObject()->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.Clear();

	//m_view->Update(39);

	//frame
	glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
}

void ComponentAgent::CompNew()
{
	flrd::ComponentEditor editor;
	editor.SetView(getObject());
	unsigned long ulval;
	std::string str;
	bool bval;
	getValue(gstCompIdStr, str);
	getValue(gstCompId, ulval);
	editor.NewId(ulval, str.empty(), false);
	//m_view->Update(39);
}

void ComponentAgent::CompAdd()
{
	flrd::ComponentEditor editor;
	editor.SetView(getObject());
	unsigned long ulval;
	std::string str;
	bool bval;
	getValue(gstCompIdStr, str);
	getValue(gstCompId, ulval);
	editor.NewId(ulval, str.empty(), true);
	//m_view->Update(39);
}

void ComponentAgent::CompReplace()
{
	flrd::ComponentEditor editor;
	editor.SetView(getObject());
	unsigned long ulval;
	std::string str;
	bool bval;
	getValue(gstCompIdStr, str);
	getValue(gstCompId, ulval);
	editor.Replace(ulval, str.empty());
	//m_view->Update(39);
}

void ComponentAgent::CompCleanBkg()
{
	flrd::ComponentEditor editor;
	editor.SetVolume(getObject()->GetCurrentVolume());
	editor.Clean(0);
	//m_view->Update(39);
}

void ComponentAgent::CompCombine()
{
	flrd::ComponentEditor editor;
	editor.SetView(getObject());
	editor.Combine();
	//m_view->Update(39);
}

//command
void ComponentAgent::LoadCmd(const wxString &filename)
{
	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);
	dlg_.m_cmd_file_text->SetValue(filename);

	m_command.clear();
	int cmd_count = 0;
	wxString str;
	std::string cmd_str = "/cmd" + std::to_string(cmd_count);
	while (fconfig.Exists(cmd_str))
	{
		flrd::CompCmdParams params;
		fconfig.SetPath(cmd_str);
		str = fconfig.Read("type", "");
		if (str == "generate" ||
			str == "clean" ||
			str == "fixate")
			params.push_back(str.ToStdString());
		else
			continue;
		long lval;
		if (fconfig.Read("iter", &lval))
		{
			params.push_back("iter"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("use_dist_field", &lval))
		{
			params.push_back("use_dist_field"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("dist_filter_size", &lval))
		{
			params.push_back("dist_filter_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("max_dist", &lval))
		{
			params.push_back("max_dist"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("diff", &lval))
		{
			params.push_back("diff"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("density", &lval))
		{
			params.push_back("density"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("density_window_size", &lval))
		{
			params.push_back("density_window_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("density_stats_size", &lval))
		{
			params.push_back("density_stats_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("cleanb", &lval))
		{
			params.push_back("cleanb"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("clean_iter", &lval))
		{
			params.push_back("clean_iter"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("clean_size_vl", &lval))
		{
			params.push_back("clean_size_vl"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("fix_size", &lval))
		{
			params.push_back("fix_size"); params.push_back(std::to_string(lval));
		}
		double dval;
		if (fconfig.Read("thresh", &dval))
		{
			params.push_back("thresh"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("dist_strength", &dval))
		{
			params.push_back("dist_strength"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("dist_thresh", &dval))
		{
			params.push_back("dist_thresh"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("falloff", &dval))
		{
			params.push_back("falloff"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("density_thresh", &dval))
		{
			params.push_back("density_thresh"); params.push_back(std::to_string(dval));
		}

		m_command.push_back(params);
		cmd_count++;
		cmd_str = "/cmd" + std::to_string(cmd_count);
	}
	//record
	int ival = m_command.size();
	dlg_.m_cmd_count_text->SetValue(wxString::Format("%d", ival));
}

void ComponentAgent::SaveCmd(const wxString &filename)
{
	if (m_command.empty())
	{
		AddCmd("generate");
	}

	wxFileConfig fconfig("", "", filename, "",
		wxCONFIG_USE_LOCAL_FILE);
	fconfig.DeleteAll();

	int cmd_count = 0;

	for (auto it = m_command.begin();
		it != m_command.end(); ++it)
	{
		if (it->empty())
			continue;
		if ((*it)[0] == "generate" ||
			(*it)[0] == "clean" ||
			(*it)[0] == "fixate")
		{
			std::string str = "/cmd" + std::to_string(cmd_count++);
			fconfig.SetPath(str);
			str = (*it)[0];
			fconfig.Write("type", wxString(str));
		}
		for (auto it2 = it->begin();
			it2 != it->end(); ++it2)
		{
			if (*it2 == "iter" ||
				*it2 == "use_dist_field" ||
				*it2 == "dist_filter_size" ||
				*it2 == "max_dist" ||
				*it2 == "diff" ||
				*it2 == "density" ||
				*it2 == "density_window_size" ||
				*it2 == "density_stats_size" ||
				*it2 == "cleanb" ||
				*it2 == "clean_iter" ||
				*it2 == "clean_size_vl" ||
				*it2 == "fix_size")
			{
				fconfig.Write(*it2, std::stoi(*(++it2)));
			}
			else if (*it2 == "thresh" ||
				*it2 == "dist_strength" ||
				*it2 == "dist_thresh" ||
				*it2 == "falloff" ||
				*it2 == "density_thresh")
			{
				fconfig.Write(*it2, std::stod(*(++it2)));
			}
		}
	}

	SaveConfig(fconfig, filename);
	dlg_.m_cmd_file_text->SetValue(filename);
}

//record
void ComponentAgent::AddCmd(const std::string &type)
{
	if (!m_command.empty())
	{
		flrd::CompCmdParams &params = m_command.back();
		if (!params.empty())
		{
			if ((params[0] == "generate" ||
				params[0] == "fixate") &&
				params[0] == type)
			{
				//replace
				m_command.pop_back();
			}
			//else do nothing
		}
	}
	bool bval;
	int ival;
	long lval;
	double dval;
	//add
	flrd::CompCmdParams params;
	if (type == "generate")
	{
		params.push_back("generate");
		getValue(gstIteration, lval); params.push_back("iter"); params.push_back(std::to_string(lval));
		getValue(gstThreshold, dval); params.push_back("thresh"); params.push_back(std::to_string(dval));
		getValue(gstUseDistField, bval); params.push_back("use_dist_field"); params.push_back(std::to_string(bval));
		getValue(gstDistFieldStrength, dval); params.push_back("dist_strength"); params.push_back(std::to_string(dval));
		getValue(gstDistFieldFilterSize, lval); params.push_back("dist_filter_size"); params.push_back(std::to_string(lval));
		getValue(gstMaxDist, lval); params.push_back("max_dist"); params.push_back(std::to_string(lval));
		getValue(gstDistFieldThresh, dval); params.push_back("dist_thresh"); params.push_back(std::to_string(dval));
		getValue(gstUseDiffusion, bval); params.push_back("diff"); params.push_back(std::to_string(bval));
		getValue(gstDiffusionFalloff, dval); params.push_back("falloff"); params.push_back(std::to_string(dval));
		getValue(gstUseDensityField, bval); params.push_back("density"); params.push_back(std::to_string(bval));
		getValue(gstDensityFieldThresh, dval); params.push_back("density_thresh"); params.push_back(std::to_string(dval));
		getValue(gstDensityVarThresh, dval); params.push_back("varth"); params.push_back(std::to_string(dval));
		getValue(gstDensityWindowSize, lval); params.push_back("density_window_size"); params.push_back(std::to_string(lval));
		getValue(gstDensityStatsSize, lval); params.push_back("density_stats_size"); params.push_back(std::to_string(lval));
		getValue(gstCleanEnable, bval); params.push_back("cleanb"); params.push_back(std::to_string(bval));
		getValue(gstCleanIteration, lval); params.push_back("clean_iter"); params.push_back(std::to_string(lval));
		getValue(gstCleanSize, lval); params.push_back("clean_size_vl"); params.push_back(std::to_string(lval));
	}
	else if (type == "clean")
	{
		params.push_back("clean");
		getValue(gstCleanIteration, lval); params.push_back("clean_iter"); params.push_back(std::to_string(lval));
		getValue(gstCleanSize, lval); params.push_back("clean_size_vl"); params.push_back(std::to_string(lval));
	}
	else if (type == "fixate")
	{
		params.push_back("fixate");
		getValue(gstFixateSize, lval); params.push_back("fix_size"); params.push_back(std::to_string(lval));
	}
	m_command.push_back(params);

	//record
	ival = m_command.size();
	dlg_.m_cmd_count_text->SetValue(wxString::Format("%d", ival));
}

void ComponentAgent::ResetCmd()
{
	m_command.clear();
	dlg_.m_record_cmd_btn->SetValue(false);
	setValue(gstRecordCmd, false);
	//m_record_cmd = false;
	//record
	//int ival = m_command.size();
	//dlg_.m_cmd_count_text->SetValue(wxString::Format("%d", ival));
}

void ComponentAgent::PlayCmd(bool use_selection, double tfactor)
{
	bool bval;
	int ival;
	long lval;
	double dval;
	//disable first
	setValue(gstFixateEnable, false);
	setValue(gstAutoUpdate, false);
	dlg_.m_auto_update_btn->SetValue(false);

	if (m_command.empty())
	{
		//the threshold factor is used to lower the threshold value for semi auto segmentation
		setValue(gstThreshScale, tfactor);
		GenerateComp(use_selection, false);
		setValue(gstThreshScale, 1.0);
		return;
	}

	for (auto it = m_command.begin();
		it != m_command.end(); ++it)
	{
		if (it->empty())
			continue;
		if ((*it)[0] == "generate")
		{
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "iter")
					setValue(gstIteration, std::stol(*(++it2)));
				else if (*it2 == "thresh")
					setValue(gstThreshold, std::stod(*(++it2)));
				else if (*it2 == "use_dist_field")
					setValue(gstUseDistField, bool(std::stoi(*(++it2))));
				else if (*it2 == "dist_strength")
					setValue(gstDistFieldStrength, std::stod(*(++it2)));
				else if (*it2 == "dist_filter_size")
					setValue(gstDistFieldFilterSize, std::stod(*(++it2)));
				else if (*it2 == "max_dist")
					setValue(gstMaxDist, std::stol(*(++it2)));
				else if (*it2 == "dist_thresh")
					setValue(gstDistFieldThresh, std::stod(*(++it2)));
				else if (*it2 == "diff")
					setValue(gstUseDiffusion, bool(std::stoi(*(++it2))));
				else if (*it2 == "falloff")
					setValue(gstDiffusionFalloff, std::stod(*(++it2)));
				else if (*it2 == "density")
					setValue(gstUseDensityField, bool(std::stoi(*(++it2))));
				else if (*it2 == "density_thresh")
					setValue(gstDensityFieldThresh, std::stod(*(++it2)));
				else if (*it2 == "varth")
					setValue(gstDensityVarThresh, std::stod(*(++it2)));
				else if (*it2 == "density_window_size")
					setValue(gstDensityWindowSize, std::stol(*(++it2)));
				else if (*it2 == "density_stats_size")
					setValue(gstDensityStatsSize, std::stol(*(++it2)));
				else if (*it2 == "cleanb")
					setValue(gstCleanEnable, bool(std::stoi(*(++it2))));
				else if (*it2 == "clean_iter")
					setValue(gstCleanIteration, std::stol(*(++it2)));
				else if (*it2 == "clean_size_vl")
					setValue(gstCleanSize, std::stol(*(++it2)));
			}
			GenerateComp(use_selection, false);
		}
		else if ((*it)[0] == "clean")
		{
			setValue(gstCleanEnable, true);
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "clean_iter")
					setValue(gstCleanIteration, std::stol(*(++it2)));
				else if (*it2 == "clean_size_vl")
					setValue(gstCleanSize, std::stol(*(++it2)));
			}
			Clean(use_selection, false);
		}
		else if ((*it)[0] == "fixate")
		{
			setValue(gstFixateEnable, true);
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "fix_size")
					setValue(gstFixateSize, std::stol(*(++it2)));
			}
			//GenerateComp(false);
			Fixate(false);
			//return;
		}
	}
	//Update();
}

void ComponentAgent::Cluster()
{
	m_in_cells.clear();
	m_out_cells.clear();

	Renderview* view = getObject();
	if (!view) return;
	fluo::VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex) return;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data) return;
	long bits;
	vd->getValue(gstBits, bits);
	void* data_data = nrrd_data->data;
	if (!data_data) return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask) return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask) return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
	{
		vd->AddEmptyLabel(0, true);
		nrrd_label = tex->get_nrrd(tex->nlabel());
	}
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label) return;

	long nx, ny, nz;
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);
	double scale;
	vd->getValue(gstIntScale, scale);
	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);

	flrd::ClusterMethod* method = 0;
	//switch method
	long clm, lval;
	double dval;
	getValue(gstClusterMethod, clm);
	if (clm == 0)
	{
		flrd::ClusterKmeans* method_kmeans = new flrd::ClusterKmeans();
		getValue(gstClusterNum, lval);
		method_kmeans->SetClnum(lval);
		getValue(gstClusterMaxIter, lval);
		method_kmeans->SetMaxiter(lval);
		method = method_kmeans;
	}
	else if (clm == 1)
	{
		flrd::ClusterExmax* method_exmax = new flrd::ClusterExmax();
		getValue(gstClusterNum, lval);
		method_exmax->SetClnum(lval);
		getValue(gstClusterMaxIter, lval);
		method_exmax->SetMaxiter(lval);
		getValue(gstClusterTol, dval);
		method_exmax->SetProbTol(dval);
		method = method_exmax;
	}
	else if (clm == 2)
	{
		flrd::ClusterDbscan* method_dbscan = new flrd::ClusterDbscan();
		getValue(gstClusterSize, lval);
		method_dbscan->SetSize(lval);
		getValue(gstClusterEps, dval);
		method_dbscan->SetEps(dval);
		method = method_dbscan;
	}

	if (!method)
		return;

	method->SetSpacings(spcx, spcy, spcz);

	//add cluster points
	size_t i, j, k;
	size_t index;
	size_t nxyz = nx * ny * nz;
	unsigned char mask_value;
	float data_value;
	unsigned int label_value;
	bool use_init_cluster = false;
	struct CmpCnt
	{
		unsigned int id;
		unsigned int size;
		bool operator<(const CmpCnt &cc) const
		{
			return size > cc.size;
		}
	};
	std::unordered_map<unsigned int, CmpCnt> init_clusters;
	std::set<CmpCnt> ordered_clusters;
	if (clm == 1)
	{
		for (index = 0; index < nxyz; ++index)
		{
			mask_value = data_mask[index];
			if (!mask_value)
				continue;
			label_value = data_label[index];
			if (!label_value)
				continue;
			auto it = init_clusters.find(label_value);
			if (it == init_clusters.end())
			{
				CmpCnt cc = { label_value, 1 };
				init_clusters.insert(std::pair<unsigned int, CmpCnt>(
					label_value, cc));
			}
			else
			{
				it->second.size++;
			}
		}
		getValue(gstClusterNum, lval);
		if (init_clusters.size() >= lval)
		{
			for (auto it = init_clusters.begin();
				it != init_clusters.end(); ++it)
				ordered_clusters.insert(it->second);
			use_init_cluster = true;
		}
	}

	for (i = 0; i < nx; ++i) for (j = 0; j < ny; ++j) for (k = 0; k < nz; ++k)
	{
		index = nx * ny*k + nx * j + i;
		mask_value = data_mask[index];
		if (mask_value)
		{
			if (bits == 8)
				data_value = ((unsigned char*)data_data)[index] / 255.0f;
			else if (bits == 16)
				data_value = ((unsigned short*)data_data)[index] * scale / 65535.0f;
			flrd::EmVec pnt = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k) };
			label_value = data_label[index];
			int cid = -1;
			if (use_init_cluster)
			{
				cid = 0;
				bool found = false;
				for (auto it = ordered_clusters.begin();
					it != ordered_clusters.end(); ++it)
				{
					if (label_value == it->id)
					{
						found = true;
						break;
					}
					cid++;
				}
				if (!found)
					cid = -1;
			}
			method->AddClusterPoint(
				pnt, data_value, cid);

			//add to list
			auto iter = m_in_cells.find(label_value);
			if (iter != m_in_cells.end())
			{
				iter->second->Inc(i, j, k, data_value);
			}
			else
			{
				flrd::Cell* cell = new flrd::Cell(label_value);
				cell->Inc(i, j, k, data_value);
				m_in_cells.insert(std::pair<unsigned int, flrd::Celp>
					(label_value, flrd::Celp(cell)));
			}
		}
	}

	if (method->Execute())
	{
		method->GenerateNewIDs(0, (void*)data_label, nx, ny, nz, true);
		m_out_cells = method->GetCellList();
		vd->GetRenderer()->clear_tex_label();
		//m_view->Update(39);
	}

	delete method;
}

void ComponentAgent::ShuffleCurVolume()
{
	//get current vd
	VolumeData* vd = getObject()->GetCurrentVolume();
	if (!vd)
		return;

	vd->IncShuffle();
	//m_view->Update(39);
}

int ComponentAgent::GetShuffle()
{
	VolumeData* vd = getObject()->GetCurrentVolume();
	if (vd)
		return vd->GetShuffle();
	return 0;
}

void ComponentAgent::CompOutput(int color_type)
{
	long lval;
	getValue(gstCompOutputType, lval);
	if (lval == 1)
		OutputMulti(color_type);
	else if (lval == 2)
		OutputRgb(color_type);
}

void ComponentAgent::OutputMulti(int color_type)
{
	Renderview* view = getObject();
	VolumeData* vd = view->GetCurrentVolume();
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!vd || !analyzer) return;
	analyzer->SetVolume(vd);
	bool bval;
	getValue(gstCompConsistent, bval);
	std::list<VolumeData*> channs;
	if (analyzer->GenMultiChannels(channs, color_type, bval))
	{
		VolumeGroup* group = 0;
		for (auto i = channs.begin(); i != channs.end(); ++i)
		{
			VolumeData* vd = *i;
			if (vd)
			{
				//m_frame->GetDataManager()->AddVolumeData(vd);
				if (i == channs.begin())
					group = view->addVolumeGroup("");
				view->addVolumeData(vd, group);
			}
		}
		//if (group)
		//{
		//	//group->SetSyncRAll(true);
		//	//group->SetSyncGAll(true);
		//	//group->SetSyncBAll(true);
		//	Color col = vd->GetGamma();
		//	group->SetGammaAll(col);
		//	col = vd->GetBrightness();
		//	group->SetBrightnessAll(col);
		//	col = vd->GetHdr();
		//	group->SetHdrAll(col);
		//}
		//m_frame->UpdateList();
		//m_frame->UpdateTree(vd->getName());
		//m_view->Update(39);
	}
}

void ComponentAgent::OutputRgb(int color_type)
{
	Renderview* view = getObject();
	VolumeData* vd = view->GetCurrentVolume();
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!vd || !analyzer) return;
	analyzer->SetVolume(vd);
	bool bval;
	getValue(gstCompConsistent, bval);
	std::list<VolumeData*> channs;
	if (analyzer->GenRgbChannels(channs, color_type, bval))
	{
		VolumeGroup* group = 0;
		for (auto i = channs.begin(); i != channs.end(); ++i)
		{
			VolumeData* vd = *i;
			if (vd)
			{
				//m_frame->GetDataManager()->AddVolumeData(vd);
				if (i == channs.begin())
					group = view->addVolumeGroup("");
				view->addVolumeData(vd, group);
			}
		}
		//if (group)
		//{
		//	//group->SetSyncRAll(true);
		//	//group->SetSyncGAll(true);
		//	//group->SetSyncBAll(true);
		//	Color col = vd->GetGamma();
		//	group->SetGammaAll(col);
		//	col = vd->GetBrightness();
		//	group->SetBrightnessAll(col);
		//	col = vd->GetHdr();
		//	group->SetHdrAll(col);
		//}
		//m_frame->UpdateList();
		//m_frame->UpdateTree(vd->getName());
		//m_view->Update(39);
	}
}

void ComponentAgent::OutputAnnotation(int type)
{
	Renderview* view = getObject();
	VolumeData* vd = view->GetCurrentVolume();
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!vd || !analyzer) return;
	analyzer->SetVolume(vd);
	Annotations* ann = glbin_annf->build();
	bool bval;
	getValue(gstCompConsistent, bval);
	if (analyzer->GenAnnotations(ann, bval, type))
	{
		ann->setRvalu(gstVolume, vd);
		ann->setValue(gstTransform, vd->GetTexture()->transform());
		//DataManager* mgr = m_frame->GetDataManager();
		//if (mgr)
		//	mgr->AddAnnotations(ann);
		view->addChild(ann);
		//m_frame->UpdateList();
		//m_frame->UpdateTree(vd->getName());
		//m_view->Update(39);
	}
}

int ComponentAgent::GetDistMatSize()
{
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	if (!analyzer) return 0;

	int gsize = analyzer->GetCompGroupSize();
	bool bval;
	getValue(gstDistAllChan, bval);
	if (bval && gsize > 1)
	{
		int matsize = 0;
		for (int i = 0; i < gsize; ++i)
		{
			flrd::CompGroup* compgroup = analyzer->GetCompGroup(i);
			if (!compgroup)
				continue;
			matsize += compgroup->celps.size();
		}
		return matsize;
	}
	else
	{
		flrd::CelpList* list = analyzer->GetCelpList();
		if (!list)
			return 0;
		return list->size();
	}
}

void ComponentAgent::DistOutput(const std::wstring &filename)
{
	int num = GetDistMatSize();
	if (num <= 0)
		return;
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	if (!analyzer)
		return;
	int gsize = analyzer->GetCompGroupSize();
	int bn = analyzer->GetBrickNum();

	//result
	std::string str;
	std::vector<std::vector<double>> rm;//result matrix
	std::vector<std::string> nl;//name list
	std::vector<int> gn;//group number
	rm.reserve(num);
	nl.reserve(num);
	if (gsize > 1)
		gn.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	//compute
	double sx, sy, sz;
	std::vector<Point> pos;
	pos.reserve(num);
	int num2 = 0;//actual number
	bool bval;
	getValue(gstDistAllChan, bval);
	if (bval && gsize > 1)
	{
		for (int i = 0; i < gsize; ++i)
		{
			flrd::CompGroup* compgroup = analyzer->GetCompGroup(i);
			if (!compgroup)
				continue;

			flrd::CellGraph &graph = compgroup->graph;
			flrd::CelpList* list = &(compgroup->celps);
			sx = list->sx;
			sy = list->sy;
			sz = list->sz;
			if (bn > 1)
				graph.ClearVisited();

			for (auto it = list->begin();
				it != list->end(); ++it)
			{
				if (bn > 1)
				{
					if (graph.Visited(it->second))
						continue;
					flrd::CelpList links;
					graph.GetLinkedComps(it->second, links,
						analyzer->GetSizeLimit());
				}

				pos.push_back(it->second->GetCenter(sx, sy, sz));
				str = std::to_string(i + 1);
				str += ":";
				str += std::to_string(it->second->Id());
				nl.push_back(str);
				gn.push_back(i);
				num2++;
			}
		}
	}
	else
	{
		flrd::CellGraph &graph = analyzer->GetCompGroup(0)->graph;
		flrd::CelpList* list = analyzer->GetCelpList();
		sx = list->sx;
		sy = list->sy;
		sz = list->sz;
		if (bn > 1)
			graph.ClearVisited();

		for (auto it = list->begin();
			it != list->end(); ++it)
		{
			if (bn > 1)
			{
				if (graph.Visited(it->second))
					continue;
				flrd::CelpList links;
				graph.GetLinkedComps(it->second, links,
					analyzer->GetSizeLimit());
			}

			pos.push_back(it->second->GetCenter(sx, sy, sz));
			str = std::to_string(it->second->Id());
			nl.push_back(str);
			num2++;
		}
	}
	double dist = 0;
	for (int i = 0; i < num2; ++i)
	{
		for (int j = i; j < num2; ++j)
		{
			dist = (pos[i] - pos[j]).length();
			rm[i][j] = dist;
			rm[j][i] = dist;
		}
	}

	bool dist_neighbor;
	getValue(gstDistNeighbor, dist_neighbor);
	long dist_neighbor_value;
	getValue(gstDistNeighborValue, dist_neighbor_value);
	bool bdist = dist_neighbor && dist_neighbor_value > 0 && dist_neighbor_value < num2 - 1;

	std::vector<double> in_group;//distances with in a group
	std::vector<double> out_group;//distance between groups
	in_group.reserve(num2*num2 / 2);
	out_group.reserve(num2*num2 / 2);
	std::vector<std::vector<int>> im;//index matrix
	if (bdist)
	{
		//sort with indices
		im.reserve(num2);
		for (size_t i = 0; i < num2; ++i)
		{
			im.push_back(std::vector<int>());
			im[i].reserve(num2);
			for (size_t j = 0; j < num2; ++j)
				im[i].push_back(j);
		}
		//copy rm
		std::vector<std::vector<double>> rm2 = rm;
		//sort
		for (size_t i = 0; i < num2; ++i)
		{
			std::sort(im[i].begin(), im[i].end(),
				[&](int ii, int jj) {return rm2[i][ii] < rm2[i][jj]; });
		}
		//fill rm
		for (size_t i = 0; i < num2; ++i) for (size_t j = 0; j < num2; ++j)
		{
			rm[i][j] = rm2[i][im[i][j]];
			if (gsize > 1 && j > 0 &&
				j <= dist_neighbor_value)
			{
				if (gn[i] == gn[im[i][j]])
					in_group.push_back(rm[i][j]);
				else
					out_group.push_back(rm[i][j]);
			}
		}
	}
	else
	{
		if (gsize > 1)
		{
			for (int i = 0; i < num2; ++i) for (int j = i + 1; j < num2; ++j)
			{
				if (gn[i] == gn[j])
					in_group.push_back(rm[i][j]);
				else
					out_group.push_back(rm[i][j]);
			}
		}
	}

	std::ofstream outfile;
	outfile.open(filename, std::ofstream::out);
	//output result matrix
	size_t dnum = bdist ? (dist_neighbor + 1) : num2;
	for (size_t i = 0; i < num2; ++i)
	{
		outfile << nl[i] << "\t";
		for (size_t j = bdist ? 1 : 0; j < dnum; ++j)
		{
			outfile << rm[i][j];
			if (j < dnum - 1)
				outfile << "\t";
		}
		outfile << "\n";
	}
	//index matrix
	if (bdist)
	{
		outfile << "\n";
		for (size_t i = 0; i < num2; ++i)
		{
			outfile << nl[i] << "\t";
			for (size_t j = bdist ? 1 : 0; j < dnum; ++j)
			{
				outfile << im[i][j] + 1;
				if (j < dnum - 1)
					outfile << "\t";
			}
			outfile << "\n";
		}
	}
	//output in_group and out_group distances
	if (gsize > 1)
	{
		outfile << "\nIn group Distances\t";
		for (size_t i = 0; i < in_group.size(); ++i)
		{
			outfile << in_group[i];
			if (i < in_group.size() - 1)
				outfile << "\t";
		}
		outfile << "\n";
		outfile << "Out group Distances\t";
		for (size_t i = 0; i < out_group.size(); ++i)
		{
			outfile << out_group[i];
			if (i < out_group.size() - 1)
				outfile << "\t";
		}
		outfile << "\n";
	}
	outfile.close();
}

void ComponentAgent::AlignCenter(flrd::Ruler* ruler)
{
	if (!ruler)
		return;
	Point center = ruler->GetCenter();
	double tx, ty, tz;
	Renderview* view = getObject();
	view->getValue(gstObjCtrX, tx);
	view->getValue(gstObjCtrY, ty);
	view->getValue(gstObjCtrZ, tz);
	view->setValue(gstObjTransX, tx - center.x());
	view->setValue(gstObjTransY, center.y() - ty);
	view->setValue(gstObjTransZ, center.z() - tz);
}

void ComponentAgent::AlignPca()
{
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	flrd::CelpList* list = analyzer->GetCelpList();
	if (!list || list->size() < 3)
		return;

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;
	flrd::RulerList rulerlist;
	flrd::Ruler ruler;
	ruler.SetRulerType(1);
	Point pt;
	for (auto it = list->begin();
		it != list->end(); ++it)
	{
		pt = it->second->GetCenter(sx, sy, sz);
		ruler.AddPoint(pt);
	}
	ruler.SetFinished();

	flrd::RulerAlign* aligner = getObject()->GetRulerAlign();
	rulerlist.push_back(&ruler);
	aligner->SetRulerList(&rulerlist);
	long lval;
	getValue(gstAlignAxisType, lval);
	aligner->AlignPca(lval);
	bool bval;
	getValue(gstAlignCenter, bval);
	if (bval) AlignCenter(&ruler);
}

bool ComponentAgent::GetCellList(flrd::CelpList &cl, bool links)
{
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	if (!analyzer) return false;
	flrd::CelpList* list = analyzer->GetCelpList();
	if (!list || list->empty())
		return false;

	cl.min = list->min;
	cl.max = list->max;
	cl.sx = list->sx;
	cl.sy = list->sy;
	cl.sz = list->sz;

	bool sel_all = false;
	std::vector<unsigned int> ids;
	std::vector<unsigned int> bids;
	int bn = analyzer->GetBrickNum();

	//selected cells are retrieved using different functions
	wxArrayInt seli = dlg_.m_output_grid->GetSelectedCols();
	if (seli.GetCount() > 0)
		sel_all = true;
	if (!sel_all)
	{
		seli = dlg_.m_output_grid->GetSelectedRows();
		dlg_.AddSelArrayInt(ids, bids, seli, bn > 1);
		//wxGridCellCoordsArray sela =
		//	m_output_grid->GetSelectionBlockBottomRight();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
		//sela = m_output_grid->GetSelectionBlockTopLeft();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
		//sela = m_output_grid->GetSelectedCells();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
	}

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;
	if (sel_all)
	{
		for (auto it = list->begin(); it != list->end(); ++it)
			FindCelps(cl, it, links);
	}
	else
	{
		for (size_t i = 0; i < ids.size(); ++i)
		{
			unsigned long long key = ids[i];
			unsigned int bid = 0;
			if (bn > 1)
			{
				key = bids[i];
				key = (key << 32) | ids[i];
				bid = bids[i];
			}
			auto it = list->find(key);
			if (it != list->end())
				FindCelps(cl, it, links);
		}
	}

	if (cl.empty())
		return false;
	return true;
}

void ComponentAgent::GetCompSelection()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::CelpList cl;
	GetCellList(cl);
	view->SetCellList(cl);
	view->setValue(gstInteractive, false);
	//m_view->Update(39);
}

void ComponentAgent::SetCompSelection(std::set<unsigned long long>& ids, int mode)
{
	if (ids.empty()) return;
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	if (!analyzer) return;

	int bn = analyzer->GetBrickNum();

	wxString str;
	unsigned long ulv;
	unsigned long long ull;
	bool flag = mode == 1;
	int lasti = -1;
	wxArrayInt sel = dlg_.m_output_grid->GetSelectedRows();
	std::set<int> rows;
	for (int i = 0; i < sel.GetCount(); ++i)
		rows.insert(sel[i]);
	for (int i = 0; i < dlg_.m_output_grid->GetNumberRows(); ++i)
	{
		str = dlg_.m_output_grid->GetCellValue(i, 0);
		if (!str.ToULong(&ulv))
			continue;
		if (bn > 1)
		{
			str = dlg_.m_output_grid->GetCellValue(i, 1);
			if (!str.ToULongLong(&ull))
				continue;
			ull = (ull << 32) | ulv;
		}
		else
			ull = ulv;
		if (ids.find(ull) != ids.end())
		{
			if (!flag)
			{
				dlg_.m_output_grid->ClearSelection();
				flag = true;
			}
			if (mode == 0)
			{
				dlg_.m_output_grid->SelectRow(i, true);
				lasti = i;
			}
			else
			{
				if (rows.find(i) != rows.end())
					dlg_.m_output_grid->DeselectRow(i);
				else
				{
					dlg_.m_output_grid->SelectRow(i, true);
					lasti = i;
				}
			}
		}
	}

	if (flag)
	{
		GetCompSelection();
		if (lasti >= 0)
			dlg_.m_output_grid->GoToCell(lasti, 0);
	}
}

void ComponentAgent::IncludeComps()
{
	Renderview* view = getObject();
	if (!view) return;
	fluo::VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!analyzer) return;

	flrd::CelpList cl;
	if (GetCellList(cl, true))
	{
		//clear complist
		flrd::CelpList *list = analyzer->GetCelpList();
		for (auto it = list->begin();
			it != list->end();)
		{
			if (cl.find(it->second->GetEId()) == cl.end())
				it = list->erase(it);
			else
				++it;
		}
		//select cl
		flrd::ComponentSelector comp_selector(vd);
		comp_selector.SelectList(cl);
		dlg_.ClearOutputGrid();
		string titles, values;
		analyzer->OutputFormHeader(titles);
		analyzer->OutputCompListStr(values, 0);
		dlg_.SetOutput(titles, values);

		cl.clear();
		view->SetCellList(cl);
		view->setValue(gstInteractive, false);
		//m_view->Update(39);

		bool bval;
		getObject()->getValue(gstPaintCount, bval);
		if (bval) glbin_agtf->getBrushToolAgent()->Update(0);
		glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
		getObject()->getValue(gstPaintColocalize, bval);
		if (bval) glbin_agtf->getColocalAgent()->Run();
	}
}

void ComponentAgent::ExcludeComps()
{
	Renderview* view = getObject();
	if (!view) return;
	fluo::VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!analyzer) return;

	flrd::CelpList cl;
	if (GetCellList(cl, true))
	{
		//clear complist
		flrd::CelpList *list = analyzer->GetCelpList();
		for (auto it = list->begin();
			it != list->end();)
		{
			if (cl.find(it->second->GetEId()) != cl.end())
				it = list->erase(it);
			else
				++it;
		}
		flrd::ComponentSelector comp_selector(vd);
		std::vector<unsigned long long> ids;
		for (auto it = list->begin();
			it != list->end(); ++it)
			ids.push_back(it->second->GetEId());
		comp_selector.Delete(ids);
		dlg_.ClearOutputGrid();
		string titles, values;
		analyzer->OutputFormHeader(titles);
		analyzer->OutputCompListStr(values, 0);
		dlg_.SetOutput(titles, values);

		cl.clear();
		view->SetCellList(cl);
		view->setValue(gstInteractive, false);
		//m_view->RefreshGL(39);

		bool bval;
		getObject()->getValue(gstPaintCount, bval);
		if (bval) glbin_agtf->getBrushToolAgent()->Update(0);
		glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
		getObject()->getValue(gstPaintColocalize, bval);
		if (bval) glbin_agtf->getColocalAgent()->Run();
	}
}

void ComponentAgent::StartTimer()
{
	bool bval;
	getValue(gstTestSpeed, bval);
	if (!bval) return;
	m_tps.push_back(std::chrono::high_resolution_clock::now());
}

void ComponentAgent::StopTimer(std::string &values, const std::string &str)
{
	bool bval;
	getValue(gstTestSpeed, bval);
	if (!bval) return;
	auto t0 = m_tps.back();
	m_tps.push_back(std::chrono::high_resolution_clock::now());
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(
			m_tps.back() - t0);

	values += str + "\t";
	values += std::to_string(time_span.count());
	values += " sec.\n";
}

bool ComponentAgent::GetIds(std::string &str, unsigned int &id, int &brick_id)
{
	std::string::size_type sz;
	try
	{
		id = std::stoul(str, &sz);
	}
	catch (...)
	{
		return false;
	}
	std::string str2;
	if (sz < str.size())
	{
		brick_id = id;
		for (size_t i = sz; i < str.size() - 1; ++i)
		{
			if (std::isdigit(static_cast<unsigned char>(str[i])))
			{
				str2 = str.substr(i);
				try
				{
					id = std::stoul(str2);
				}
				catch (...)
				{
					return false;
				}
				return true;
			}
		}
	}
	brick_id = 0;
	return true;
}

void ComponentAgent::FindCelps(flrd::CelpList &list,
	flrd::CelpListIter &it, bool links)
{
	flrd::ComponentAnalyzer* analyzer = getObject()->GetCompAnalyzer();
	if (!analyzer) return;

	list.insert(std::pair<unsigned long long, flrd::Celp>
		(it->second->GetEId(), it->second));

	if (links)
	{
		flrd::CellGraph* graph = analyzer->GetCellGraph();
		graph->ClearVisited();
		flrd::CelpList links;
		if (graph->GetLinkedComps(it->second, links,
			analyzer->GetSizeLimit()))
		{
			for (auto it2 = links.begin();
				it2 != links.end(); ++it2)
			{
				list.insert(std::pair<unsigned long long, flrd::Celp>
					(it2->second->GetEId(), it2->second));
			}
		}
	}
}

//update functions
void ComponentAgent::OnAutoUpdate(Event& event)
{
	bool bval;
	getValue(gstAutoUpdate, bval);
	if (bval)
		GenerateComp();
}

void ComponentAgent::OnUseDistField(Event& event)
{
	bool bval;
	getValue(gstUseDistField, bval);
	if (bval)
	{
		dlg_.m_dist_strength_sldr->Enable();
		dlg_.m_dist_strength_text->Enable();
		dlg_.m_dist_filter_size_sldr->Enable();
		dlg_.m_dist_filter_size_text->Enable();
		dlg_.m_max_dist_sldr->Enable();
		dlg_.m_max_dist_text->Enable();
		dlg_.m_dist_thresh_sldr->Enable();
		dlg_.m_dist_thresh_text->Enable();
	}
	else
	{
		dlg_.m_dist_strength_sldr->Disable();
		dlg_.m_dist_strength_text->Disable();
		dlg_.m_dist_filter_size_sldr->Disable();
		dlg_.m_dist_filter_size_text->Disable();
		dlg_.m_max_dist_sldr->Disable();
		dlg_.m_max_dist_text->Disable();
		dlg_.m_dist_thresh_sldr->Disable();
		dlg_.m_dist_thresh_text->Disable();
	}
	OnAutoUpdate(event);
}

void ComponentAgent::OnUseDiffusion(Event& event)
{
	bool bval;
	getValue(gstUseDiffusion, bval);
	if (bval)
	{
		dlg_.m_falloff_sldr->Enable();
		dlg_.m_falloff_text->Enable();
	}
	else
	{
		dlg_.m_falloff_sldr->Disable();
		dlg_.m_falloff_text->Disable();
	}
	OnAutoUpdate(event);
}

void ComponentAgent::OnUseDensityField(Event& event)
{
	bool bval;
	getValue(gstUseDensityField, bval);
	if (bval)
	{
		dlg_.m_density_sldr->Enable();
		dlg_.m_density_text->Enable();
		dlg_.m_varth_sldr->Enable();
		dlg_.m_varth_text->Enable();
		dlg_.m_density_window_size_sldr->Enable();
		dlg_.m_density_window_size_text->Enable();
		dlg_.m_density_stats_size_sldr->Enable();
		dlg_.m_density_stats_size_text->Enable();
	}
	else
	{
		dlg_.m_density_sldr->Disable();
		dlg_.m_density_text->Disable();
		dlg_.m_varth_sldr->Disable();
		dlg_.m_varth_text->Disable();
		dlg_.m_density_window_size_sldr->Disable();
		dlg_.m_density_window_size_text->Disable();
		dlg_.m_density_stats_size_sldr->Disable();
		dlg_.m_density_stats_size_text->Disable();
	}
	OnAutoUpdate(event);
}

void ComponentAgent::OnFixateEnable(Event& event)
{
	bool bval;
	getValue(gstFixateEnable, bval);
	if (bval)
	{
		dlg_.m_fix_update_btn->Enable();
		dlg_.m_fix_size_sldr->Enable();
		dlg_.m_fix_size_text->Enable();
	}
	else
	{
		dlg_.m_fix_update_btn->Disable();
		dlg_.m_fix_size_sldr->Disable();
		dlg_.m_fix_size_text->Disable();
	}

	if (bval)
		Fixate(true);
}

void ComponentAgent::OnFixateSize(Event& event)
{
	OnAutoUpdate(event);
	bool bval;
	getValue(gstRecordCmd, bval);
	if (bval)
		AddCmd("fixate");
}

void ComponentAgent::OnCleanEnable(Event& event)
{
	bool bval;
	getValue(gstCleanEnable, bval);
	if (bval)
	{
		dlg_.m_clean_btn->Enable();
		dlg_.m_clean_iter_sldr->Enable();
		dlg_.m_clean_iter_text->Enable();
		dlg_.m_clean_limit_sldr->Enable();
		dlg_.m_clean_limit_text->Enable();
	}
	else
	{
		dlg_.m_clean_btn->Disable();
		dlg_.m_clean_iter_sldr->Disable();
		dlg_.m_clean_iter_text->Disable();
		dlg_.m_clean_limit_sldr->Disable();
		dlg_.m_clean_limit_text->Disable();
	}
	OnAutoUpdate(event);
}

void ComponentAgent::OnClusterMethod(Event& event)
{
	long lval;
	getValue(gstClusterMethod, lval);
	switch (lval)
	{
	case 0:
		dlg_.m_cluster_method_kmeans_rd->SetValue(true);
		dlg_.m_cluster_method_exmax_rd->SetValue(false);
		dlg_.m_cluster_method_dbscan_rd->SetValue(false);
		break;
	case 1:
		dlg_.m_cluster_method_kmeans_rd->SetValue(false);
		dlg_.m_cluster_method_exmax_rd->SetValue(true);
		dlg_.m_cluster_method_dbscan_rd->SetValue(false);
		break;
	case 2:
		dlg_.m_cluster_method_kmeans_rd->SetValue(false);
		dlg_.m_cluster_method_exmax_rd->SetValue(false);
		dlg_.m_cluster_method_dbscan_rd->SetValue(true);
		break;
	}
	switch (lval)
	{
	case 0:
	case 1:
		dlg_.m_cluster_clnum_sldr->Enable();
		dlg_.m_cluster_clnum_text->Enable();
		dlg_.m_cluster_size_sldr->Disable();
		dlg_.m_cluster_size_text->Disable();
		dlg_.m_cluster_eps_sldr->Disable();
		dlg_.m_cluster_eps_text->Disable();
		break;
	case 2:
		dlg_.m_cluster_clnum_sldr->Disable();
		dlg_.m_cluster_clnum_text->Disable();
		dlg_.m_cluster_size_sldr->Enable();
		dlg_.m_cluster_size_text->Enable();
		dlg_.m_cluster_eps_sldr->Enable();
		dlg_.m_cluster_eps_text->Enable();
		break;
	}
}

void ComponentAgent::OnUseMin(Event& event)
{
	bool bval;
	getValue(gstUseMin, bval);
	dlg_.m_analysis_min_spin->Enable(bval);
}

void ComponentAgent::OnUseMax(Event& event)
{
	bool bval;
	getValue(gstUseMax, bval);
	dlg_.m_analysis_max_spin->Enable(bval);
}