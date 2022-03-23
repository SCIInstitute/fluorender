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
#include <AnnotationFactory.hpp>
#include <VolumeData.hpp>
#include <CompAnalyzer.h>
#include <CompSelector.h>
#include <CompEditor.h>
#include <cctype>

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
	//update ui
	getValue(gstUseSelection, bval);
	dlg_.m_use_sel_chk->SetValue(bval);
	//comp generate page
	dlg_.m_iter_text->SetValue(wxString::Format("%d", m_iter));
	dlg_.m_thresh_text->SetValue(wxString::Format("%.3f", m_thresh));
	//dist
	dlg_.m_use_dist_field_check->SetValue(m_use_dist_field);
	EnableUseDistField(m_use_dist_field);
	dlg_.m_dist_strength_text->SetValue(wxString::Format("%.3f", m_dist_strength));
	dlg_.m_dist_filter_size_text->SetValue(wxString::Format("%d", m_dist_filter_size));
	dlg_.m_max_dist_text->SetValue(wxString::Format("%d", m_max_dist));
	dlg_.m_dist_thresh_text->SetValue(wxString::Format("%.3f", m_dist_thresh));
	dlg_.m_diff_check->SetValue(m_diff);
	EnableDiff(m_diff);
	dlg_.m_falloff_text->SetValue(wxString::Format("%.3f", m_falloff));
	//m_size_check->SetValue(m_size);
	EnableSize(m_size);
	//m_size_text->SetValue(wxString::Format("%d", m_size_lm));
	EnableDensity(m_density);
	dlg_.m_density_check->SetValue(m_density);
	dlg_.m_density_text->SetValue(wxString::Format("%.3f", m_density_thresh));
	dlg_.m_varth_text->SetValue(wxString::Format("%.4f", m_varth));
	dlg_.m_density_window_size_text->SetValue(wxString::Format("%d", m_density_window_size));
	dlg_.m_density_stats_size_text->SetValue(wxString::Format("%d", m_density_stats_size));
	//fixate
	dlg_.m_fixate_check->SetValue(m_fixate);
	EnableFixate(m_fixate);
	dlg_.m_fix_size_text->SetValue(wxString::Format("%d", m_fix_size));
	//clean
	EnableClean(m_clean);
	dlg_.m_clean_check->SetValue(m_clean);
	dlg_.m_clean_iter_text->SetValue(wxString::Format("%d", m_clean_iter));
	dlg_.m_clean_limit_text->SetValue(wxString::Format("%d", m_clean_size_vl));
	//record
	ival = m_command.size();
	dlg_.m_cmd_count_text->SetValue(wxString::Format("%d", ival));

	//cluster page
	dlg_.m_cluster_method_exmax_rd->SetValue(m_cluster_method_exmax);
	dlg_.m_cluster_method_dbscan_rd->SetValue(m_cluster_method_dbscan);
	dlg_.m_cluster_method_kmeans_rd->SetValue(m_cluster_method_kmeans);
	//parameters
	dlg_.m_cluster_clnum_text->SetValue(wxString::Format("%d", m_cluster_clnum));
	dlg_.m_cluster_maxiter_text->SetValue(wxString::Format("%d", m_cluster_maxiter));
	dlg_.m_cluster_tol_text->SetValue(wxString::Format("%.2f", m_cluster_tol));
	dlg_.m_cluster_size_text->SetValue(wxString::Format("%d", m_cluster_size));
	dlg_.m_cluster_eps_text->SetValue(wxString::Format("%.1f", m_cluster_eps));

	//selection
	if (m_use_min)
	{
		dlg_.m_analysis_min_check->SetValue(true);
		dlg_.m_analysis_min_spin->Enable();
	}
	else
	{
		dlg_.m_analysis_min_check->SetValue(false);
		dlg_.m_analysis_min_spin->Disable();
	}
	dlg_.m_analysis_min_spin->SetValue(m_min_num);
	if (m_use_max)
	{
		dlg_.m_analysis_max_check->SetValue(true);
		dlg_.m_analysis_max_spin->Enable();
	}
	else
	{
		dlg_.m_analysis_max_check->SetValue(false);
		dlg_.m_analysis_max_spin->Disable();
	}
	dlg_.m_analysis_max_spin->SetValue(m_max_num);

	//options
	dlg_.m_consistent_check->SetValue(m_consistent);
	dlg_.m_colocal_check->SetValue(m_colocal);

	//output type
	dlg_.m_output_multi_rb->SetValue(false);
	dlg_.m_output_rgb_rb->SetValue(false);
	if (m_output_type == 1)
		dlg_.m_output_multi_rb->SetValue(true);
	else if (m_output_type == 2)
		dlg_.m_output_rgb_rb->SetValue(true);

	dlg_.m_dist_neighbor_check->SetValue(m_use_dist_neighbor);
	dlg_.m_dist_neighbor_sldr->Enable(m_use_dist_neighbor);
	dlg_.m_dist_neighbor_text->Enable(m_use_dist_neighbor);
	dlg_.m_dist_all_chan_check->SetValue(m_use_dist_allchan);

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

	//basic settings
	fconfig.Read("use_sel", &m_use_sel);
	fconfig.Read("iter", &m_iter);
	fconfig.Read("thresh", &m_thresh);
	fconfig.Read("use_dist_field", &m_use_dist_field);
	fconfig.Read("dist_strength", &m_dist_strength);
	fconfig.Read("dist_filter_size", &m_dist_filter_size);
	fconfig.Read("max_dist", &m_max_dist);
	fconfig.Read("dist_thresh", &m_dist_thresh);
	fconfig.Read("diff", &m_diff);
	fconfig.Read("falloff", &m_falloff);
	fconfig.Read("size", &m_size);
	fconfig.Read("size_lm", &m_size_lm);
	fconfig.Read("density", &m_density);
	fconfig.Read("density_thresh", &m_density_thresh);
	fconfig.Read("varth", &m_varth);
	fconfig.Read("density_window_size", &m_density_window_size);
	fconfig.Read("density_stats_size", &m_density_stats_size);
	fconfig.Read("clean", &m_clean);
	fconfig.Read("clean_iter", &m_clean_iter);
	fconfig.Read("clean_size_vl", &m_clean_size_vl);

	//cluster
	fconfig.Read("cluster_method_kmeans", &m_cluster_method_kmeans);
	fconfig.Read("cluster_method_exmax", &m_cluster_method_exmax);
	fconfig.Read("cluster_method_dbscan", &m_cluster_method_dbscan);
	//parameters
	fconfig.Read("cluster_clnum", &m_cluster_clnum);
	fconfig.Read("cluster_maxiter", &m_cluster_maxiter);
	fconfig.Read("cluster_tol", &m_cluster_tol);
	fconfig.Read("cluster_size", &m_cluster_size);
	fconfig.Read("cluster_eps", &m_cluster_eps);

	//selection
	fconfig.Read("use_min", &m_use_min);
	fconfig.Read("min_num", &m_min_num);
	fconfig.Read("use_max", &m_use_max);
	fconfig.Read("max_num", &m_max_num);
	//colocalization
	fconfig.Read("colocal", &m_colocal);
	//output
	fconfig.Read("output_type", &m_output_type);

	//m_load_settings_text->SetValue(filename);
}

void ComponentAgent::SaveSettings(const wxString &filename)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_component_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	//comp generate settings
	fconfig.Write("use_sel", m_use_sel);
	fconfig.Write("iter", m_iter);
	fconfig.Write("thresh", m_thresh);
	fconfig.Write("use_dist_field", m_use_dist_field);
	fconfig.Write("dist_strength", m_dist_strength);
	fconfig.Write("dist_filter_size", m_dist_filter_size);
	fconfig.Write("max_dist", m_max_dist);
	fconfig.Write("dist_thresh", m_dist_thresh);
	fconfig.Write("diff", m_diff);
	fconfig.Write("falloff", m_falloff);
	fconfig.Write("size", m_size);
	fconfig.Write("size_lm", m_size_lm);
	fconfig.Write("density", m_density);
	fconfig.Write("density_thresh", m_density_thresh);
	fconfig.Write("varth", m_varth);
	fconfig.Write("density_window_size", m_density_window_size);
	fconfig.Write("density_stats_size", m_density_stats_size);
	fconfig.Write("clean", m_clean);
	fconfig.Write("clean_iter", m_clean_iter);
	fconfig.Write("clean_size_vl", m_clean_size_vl);

	//cluster
	fconfig.Write("cluster_method_kmeans", m_cluster_method_kmeans);
	fconfig.Write("cluster_method_exmax", m_cluster_method_exmax);
	fconfig.Write("cluster_method_dbscan", m_cluster_method_dbscan);
	//parameters
	fconfig.Write("cluster_clnum", m_cluster_clnum);
	fconfig.Write("cluster_maxiter", m_cluster_maxiter);
	fconfig.Write("cluster_tol", m_cluster_tol);
	fconfig.Write("cluster_size", m_cluster_size);
	fconfig.Write("cluster_eps", m_cluster_eps);

	//selection
	fconfig.Write("use_min", m_use_min);
	fconfig.Write("min_num", m_min_num);
	fconfig.Write("use_max", m_use_max);
	fconfig.Write("max_num", m_max_num);
	//colocalization
	fconfig.Write("colocal", m_colocal);
	//output
	fconfig.Write("output_type", m_output_type);

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

void ComponentAgent::Analyze(bool sel)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	int bn = vd->GetAllBrickNum();
	m_prog_bit = 97.0f / float(bn * 2 + (m_consistent ? 1 : 0));
	m_prog = 0.0f;

	//boost::signals2::connection connection =
	//	m_comp_analyzer.m_sig_progress.connect(std::bind(
	//	&ComponentDlg::UpdateProgress, this));

	m_comp_analyzer.SetVolume(vd);
	if (m_colocal)
	{
		m_comp_analyzer.ClearCoVolumes();
		fluo::VolumeList list = m_view->GetVolList();
		for (auto vdi : list)
		{
			if (vdi != vd)
				m_comp_analyzer.AddCoVolume(vdi);
		}
	}
	m_comp_analyzer.Analyze(sel, m_consistent, m_colocal);

	if (m_consistent)
	{
		//invalidate label mask in gpu
		vd->GetRenderer()->clear_tex_label();
		m_view->Update(39);
	}

	if (m_comp_analyzer.GetListSize() > 10000)
	{
		wxFileDialog *fopendlg = new wxFileDialog(
			this, "Save Analysis Data", "", "",
			"Text file (*.txt)|*.txt",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		int rval = fopendlg->ShowModal();
		if (rval == wxID_OK)
		{
			wxString filename = fopendlg->GetPath();
			string str = filename.ToStdString();
			m_comp_analyzer.OutputCompListFile(str, 1);
		}
		if (fopendlg)
			delete fopendlg;
	}
	else
	{
		string titles, values;
		m_comp_analyzer.OutputFormHeader(titles);
		m_comp_analyzer.OutputCompListStr(values, 0);
		wxString str1(titles), str2(values);
		SetOutput(str1, str2);
	}

	//connection.disconnect();
}

void ComponentAgent::GenerateComp()
{
	Renderview* view = getObject();
	if (!view)
		return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd)
		return;
	bool use_sel, command;
	getValue(gstUseSelection, use_sel);
	getValue(gstRunCmd, command);

	int clean_iter = m_clean_iter;
	int clean_size = m_clean_size_vl;
	if (!m_clean)
	{
		clean_iter = 0;
		clean_size = 0;
	}

	//get brick number
	int bn = vd->GetAllBrickNum();
	double scale;
	vd->getValue(gstIntScale, scale);

	flrd::ComponentGenerator cg(vd);
	boost::signals2::connection preconn =
		cg.prework.connect(std::bind(
			&ComponentDlg::StartTimer, this, std::placeholders::_1));
	boost::signals2::connection postconn =
		cg.postwork.connect(std::bind(
			&ComponentDlg::StopTimer, this, std::placeholders::_1));
	m_titles.Clear();
	m_values.Clear();
	m_tps.clear();
	m_tps.push_back(std::chrono::high_resolution_clock::now());

	cg.SetUseMask(use_sel);

	vd->AddEmptyMask(cg.GetUseMask() ? 2 : 1, true);//select all if no mask, otherwise keep
	if (m_fixate && vd->GetLabel(false))
	{
		vd->LoadLabel2();
		cg.SetIDBit(m_fix_size);
	}
	else
	{
		vd->AddEmptyLabel(0, !use_sel);
		cg.ShuffleID();
	}

	if (m_use_dist_field)
	{
		if (m_density)
		{
			cg.DistDensityField(
				m_diff, m_iter,
				m_thresh*m_tfactor,
				m_falloff,
				m_dist_filter_size,
				m_max_dist,
				m_dist_thresh,
				m_dist_strength,
				m_density_window_size,
				m_density_stats_size,
				m_density_thresh,
				m_varth,
				scale);
		}
		else
		{
			cg.DistGrow(
				m_diff, m_iter,
				m_thresh*m_tfactor,
				m_falloff,
				m_dist_filter_size,
				m_max_dist,
				m_dist_thresh,
				scale,
				m_dist_strength);
		}
	}
	else
	{
		if (m_density)
		{
			cg.DensityField(
				m_density_window_size,
				m_density_stats_size,
				m_diff, m_iter,
				m_thresh*m_tfactor,
				m_falloff,
				m_density_thresh,
				m_varth,
				scale);
		}
		else
		{
			cg.Grow(
				m_diff,
				m_iter,
				m_thresh*m_tfactor,
				m_falloff,
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
	if (m_test_speed)
	{
		m_titles += "Function\t";
		m_titles += "Time\n";
		m_values += "Total\t";
	}
	else
	{
		m_titles += "Total time\n";
	}
	m_values += wxString::Format("%.4f", time_span.count());
	m_values += " sec.\n";
	SetOutput(m_titles, m_values);

	//update
	//m_view->Update(39);

	if (command && m_record_cmd)
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
	if (bval) glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->Update(0);
	glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->UpdateUndoRedo();
	getObject->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
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
	if (bval) glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->Update(0);
	glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->UpdateUndoRedo();
	getObject->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
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
	if (bval) glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->Update(0);
	glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->UpdateUndoRedo();
	getObject->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
}

void ComponentAgent::CompAll()
{
	//get current vd
	VolumeData* vd = getObject()->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.All();

	//m_view->Update(39);

	getObject()->getValue(gstPaintCount, bval);
	if (bval) glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->Update(0);
	glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->UpdateUndoRedo();
	getObject->getValue(gstPaintColocalize, bval);
	if (bval) glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
}

void ComponentAgent::CompClear()
{
	//get current vd
	VolumeData* vd = getObject()->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.Clear();

	//m_view->Update(39);

	//frame
	glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->UpdateUndoRedo();
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
	//add
	flrd::CompCmdParams params;
	if (type == "generate")
	{
		params.push_back("generate");
		params.push_back("iter"); params.push_back(std::to_string(m_iter));
		params.push_back("thresh"); params.push_back(std::to_string(m_thresh));
		params.push_back("use_dist_field"); params.push_back(std::to_string(m_use_dist_field));
		params.push_back("dist_strength"); params.push_back(std::to_string(m_dist_strength));
		params.push_back("dist_filter_size"); params.push_back(std::to_string(m_dist_filter_size));
		params.push_back("max_dist"); params.push_back(std::to_string(m_max_dist));
		params.push_back("dist_thresh"); params.push_back(std::to_string(m_dist_thresh));
		params.push_back("diff"); params.push_back(std::to_string(m_diff));
		params.push_back("falloff"); params.push_back(std::to_string(m_falloff));
		params.push_back("density"); params.push_back(std::to_string(m_density));
		params.push_back("density_thresh"); params.push_back(std::to_string(m_density_thresh));
		params.push_back("varth"); params.push_back(std::to_string(m_varth));
		params.push_back("density_window_size"); params.push_back(std::to_string(m_density_window_size));
		params.push_back("density_stats_size"); params.push_back(std::to_string(m_density_stats_size));
		params.push_back("cleanb"); params.push_back(std::to_string(m_clean));
		params.push_back("clean_iter"); params.push_back(std::to_string(m_clean_iter));
		params.push_back("clean_size_vl"); params.push_back(std::to_string(m_clean_size_vl));
	}
	else if (type == "clean")
	{
		params.push_back("clean");
		params.push_back("clean_iter"); params.push_back(std::to_string(m_clean_iter));
		params.push_back("clean_size_vl"); params.push_back(std::to_string(m_clean_size_vl));
	}
	else if (type == "fixate")
	{
		params.push_back("fixate");
		params.push_back("fix_size"); params.push_back(std::to_string(m_fix_size));
	}
	m_command.push_back(params);

	//record
	int ival = m_command.size();
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

void ComponentAgent::PlayCmd(double tfactor)
{
	//disable first
	m_fixate = false;
	m_auto_update = false;
	m_auto_update_btn->SetValue(false);

	if (m_command.empty())
	{
		//the threshold factor is used to lower the threshold value for semi auto segmentation
		m_tfactor = tfactor;
		GenerateComp(use_sel, false);
		m_tfactor = 1.0;
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
					m_iter = std::stoi(*(++it2));
				else if (*it2 == "thresh")
					m_thresh = std::stod(*(++it2));
				else if (*it2 == "use_dist_field")
					m_use_dist_field = std::stoi(*(++it2));
				else if (*it2 == "dist_strength")
					m_dist_strength = std::stod(*(++it2));
				else if (*it2 == "dist_filter_size")
					m_dist_filter_size = std::stod(*(++it2));
				else if (*it2 == "max_dist")
					m_max_dist = std::stoi(*(++it2));
				else if (*it2 == "dist_thresh")
					m_dist_thresh = std::stod(*(++it2));
				else if (*it2 == "diff")
					m_diff = std::stoi(*(++it2));
				else if (*it2 == "falloff")
					m_falloff = std::stod(*(++it2));
				else if (*it2 == "density")
					m_density = std::stoi(*(++it2));
				else if (*it2 == "density_thresh")
					m_density_thresh = std::stod(*(++it2));
				else if (*it2 == "varth")
					m_varth = std::stod(*(++it2));
				else if (*it2 == "density_window_size")
					m_density_window_size = std::stoi(*(++it2));
				else if (*it2 == "density_stats_size")
					m_density_stats_size = std::stoi(*(++it2));
				else if (*it2 == "cleanb")
					m_clean = std::stoi(*(++it2));
				else if (*it2 == "clean_iter")
					m_clean_iter = std::stoi(*(++it2));
				else if (*it2 == "clean_size_vl")
					m_clean_size_vl = std::stoi(*(++it2));
			}
			GenerateComp(use_sel, false);
		}
		else if ((*it)[0] == "clean")
		{
			m_clean = true;
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "clean_iter")
					m_clean_iter = std::stoi(*(++it2));
				else if (*it2 == "clean_size_vl")
					m_clean_size_vl = std::stoi(*(++it2));
			}
			Clean(use_sel, false);
		}
		else if ((*it)[0] == "fixate")
		{
			m_fixate = true;
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "fix_size")
					m_fix_size = std::stoi(*(++it2));
			}
			//GenerateComp(false);
			Fixate(false);
			//return;
		}
	}
	Update();
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
	if (!analyzer) return;

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

void ComponentAgent::DistOutput()
{

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
	if (bval)
		dlg_.m_analysis_min_spin->Enable();
	else
		dlg_.m_analysis_min_spin->Disable();
}

void ComponentAgent::OnUseMax(Event& event)
{
	bool bval;
	getValue(gstUseMax, bval);
	if (bval)
		dlg_.m_analysis_max_spin->Enable();
	else
		dlg_.m_analysis_max_spin->Disable();
}