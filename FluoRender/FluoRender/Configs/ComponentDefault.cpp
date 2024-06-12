/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#include <ComponentDefault.h>
#include <Names.h>
#include <compatibility.h>
#include <CompGenerator.h>
#include <CompSelector.h>
#include <CompAnalyzer.h>
#include <Clusterizer.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>

ComponentDefault::ComponentDefault()
{
	//defaults
	//comp generate page
	m_use_sel = false;
	m_use_ml = false;
	m_iter = 50;
	m_thresh = 0.5;
	m_tfactor = 1.0;
	m_use_dist_field = false;
	m_dist_strength = 0.5;
	m_max_dist = 30;
	m_dist_thresh = 0.25;
	m_dist_filter_size = 3;
	m_diff = false;
	m_falloff = 0.01;
	m_size = false;
	m_size_lm = 100;
	m_density = false;
	m_density_thresh = 1.0;
	m_varth = 0.0001;
	m_density_window_size = 5;
	m_density_stats_size = 15;
	m_fixate = false;
	m_fix_size = 50;
	m_grow_fixed = 1;
	m_clean = false;
	m_clean_iter = 5;
	m_clean_size_vl = 5;
	m_fill_border = 0.1;

	//cluster
	m_cluster_method = 0;
	m_cluster_clnum = 2;
	m_cluster_maxiter = 200;
	m_cluster_tol = 0.9f;
	m_cluster_size = 60;
	m_cluster_eps = 2.5;

	//selection
	m_use_min = false;
	m_min_num = 0;
	m_use_max = false;
	m_max_num = 0;

	//analyzer
	m_slimit = 5;
	m_colocal = false;
	m_consistent = false;
	m_channel_type = 1;
	m_color_type = 1;
	m_annot_type = 1;
	//distance
	m_use_dist_neighbor = false;
	m_dist_neighbor_num = 1;
	m_use_dist_allchan = false;

	//update
	m_auto_update = false;

	//record
	m_record_cmd = false;
}

ComponentDefault::~ComponentDefault()
{

}

void ComponentDefault::Read(const std::string& filename)
{
	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	Read(fconfig);
}

void ComponentDefault::Save(const std::string& filename)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "component_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	Save(fconfig);
	SaveConfig(fconfig, filename);
}

void ComponentDefault::Read(wxFileConfig& f)
{
	if (f.Exists("/comp default"))
		f.SetPath("/comp default");

	//basic settings
	f.Read("use_sel", &m_use_sel);
	f.Read("use_ml", &m_use_ml);
	f.Read("iter", &m_iter);
	f.Read("thresh", &m_thresh);
	f.Read("use_dist_field", &m_use_dist_field);
	f.Read("dist_strength", &m_dist_strength);
	f.Read("dist_filter_size", &m_dist_filter_size);
	f.Read("max_dist", &m_max_dist);
	f.Read("dist_thresh", &m_dist_thresh);
	f.Read("diff", &m_diff);
	f.Read("falloff", &m_falloff);
	f.Read("size", &m_size);
	f.Read("size_lm", &m_size_lm);
	f.Read("density", &m_density);
	f.Read("density_thresh", &m_density_thresh);
	f.Read("varth", &m_varth);
	f.Read("density_window_size", &m_density_window_size);
	f.Read("density_stats_size", &m_density_stats_size);
	f.Read("clean", &m_clean);
	f.Read("clean_iter", &m_clean_iter);
	f.Read("clean_size_vl", &m_clean_size_vl);
	f.Read("grow_fixed", &m_grow_fixed);
	f.Read("fill_border", &m_fill_border);

	//cluster
	f.Read("cluster_method", &m_cluster_method);
	//parameters
	f.Read("cluster_clnum", &m_cluster_clnum);
	f.Read("cluster_maxiter", &m_cluster_maxiter);
	f.Read("cluster_tol", &m_cluster_tol);
	f.Read("cluster_size", &m_cluster_size);
	f.Read("cluster_eps", &m_cluster_eps);

	//selection
	f.Read("use_min", &m_use_min);
	f.Read("min_num", &m_min_num);
	f.Read("use_max", &m_use_max);
	f.Read("max_num", &m_max_num);

	//analyzer
	f.Read("size limit", &m_slimit);
	f.Read("colocal", &m_colocal);
	f.Read("consistent", &m_consistent);
	f.Read("channel type", &m_channel_type);
	f.Read("color type", &m_color_type);
	f.Read("annot type", &m_annot_type);
	//distance
	f.Read("use_dist_neighbor", &m_use_dist_neighbor);
	f.Read("dist_neighbor", &m_dist_neighbor_num);
	f.Read("use_dist_allchan", &m_use_dist_allchan);

	////auto update
	//f.Read("auto_update", &m_auto_update);

	////record
	//f.Read("record_cmd", &m_record_cmd);
}

void ComponentDefault::Save(wxFileConfig& f)
{
	f.SetPath("/comp default");

	//comp generate settings
	f.Write("use_sel", m_use_sel);
	f.Write("use_ml", m_use_ml);
	f.Write("iter", m_iter);
	f.Write("thresh", m_thresh);
	f.Write("use_dist_field", m_use_dist_field);
	f.Write("dist_strength", m_dist_strength);
	f.Write("dist_filter_size", m_dist_filter_size);
	f.Write("max_dist", m_max_dist);
	f.Write("dist_thresh", m_dist_thresh);
	f.Write("diff", m_diff);
	f.Write("falloff", m_falloff);
	f.Write("size", m_size);
	f.Write("size_lm", m_size_lm);
	f.Write("density", m_density);
	f.Write("density_thresh", m_density_thresh);
	f.Write("varth", m_varth);
	f.Write("density_window_size", m_density_window_size);
	f.Write("density_stats_size", m_density_stats_size);
	f.Write("clean", m_clean);
	f.Write("clean_iter", m_clean_iter);
	f.Write("clean_size_vl", m_clean_size_vl);
	f.Write("grow_fixed", m_grow_fixed);
	f.Write("fill_border", m_fill_border);

	//cluster
	f.Write("cluster_method", m_cluster_method);
	//parameters
	f.Write("cluster_clnum", m_cluster_clnum);
	f.Write("cluster_maxiter", m_cluster_maxiter);
	f.Write("cluster_tol", m_cluster_tol);
	f.Write("cluster_size", m_cluster_size);
	f.Write("cluster_eps", m_cluster_eps);

	//selection
	f.Write("use_min", m_use_min);
	f.Write("min_num", m_min_num);
	f.Write("use_max", m_use_max);
	f.Write("max_num", m_max_num);

	//analyzer
	f.Write("size limit", m_slimit);
	f.Write("colocal", m_colocal);
	f.Write("consistent", m_consistent);
	f.Write("channel type", m_channel_type);
	f.Write("color type", m_color_type);
	f.Write("annot type", m_annot_type);
	//distance
	f.Write("use_dist_neighbor", m_use_dist_neighbor);
	f.Write("dist_neighbor", m_dist_neighbor_num);
	f.Write("use_dist_allchan", m_use_dist_allchan);

	//auto update
	f.Write("auto_update", m_auto_update);

	//record
	f.Write("record_cmd", m_record_cmd);
}

void ComponentDefault::Set(flrd::ComponentGenerator* cg)
{
	if (!cg)
		return;

	m_use_sel = cg->GetUseSel();
	m_use_ml = cg->GetUseMl();
	m_iter = cg->GetIter();
	m_thresh = cg->GetThresh();
	m_tfactor = cg->GetTFactor();
	m_use_dist_field = cg->GetUseDistField();
	m_dist_strength = cg->GetDistStrength();
	m_dist_filter_size = cg->GetDistFilterSize();
	m_max_dist = cg->GetMaxDist();
	m_dist_thresh = cg->GetDistThresh();
	m_diff = cg->GetDiffusion();
	m_falloff = cg->GetFalloff();
	m_size = cg->GetSize();
	m_size_lm = cg->GetSizeLimit();
	m_density = cg->GetDensity();
	m_density_thresh = cg->GetDensityThresh();
	m_varth = cg->GetVarThresh();
	m_density_window_size = cg->GetDensityWinSize();
	m_density_stats_size = cg->GetDensityStatSize();
	m_fixate = cg->GetFixate();
	m_grow_fixed = cg->GetGrowFixed();
	m_clean = cg->GetClean();
	m_clean_iter = cg->GetCleanIter();
	m_clean_size_vl = cg->GetCleanSize();
	m_fill_border = cg->GetFillBorder();
}

void ComponentDefault::Apply(flrd::ComponentGenerator* cg)
{
	if (!cg)
		return;

	cg->SetUseSel(m_use_sel);
	cg->SetUseMl(m_use_ml);
	cg->SetIter(m_iter);
	cg->SetThresh(m_thresh);
	cg->SetTFactor(m_tfactor);
	cg->SetUseDistField(m_use_dist_field);
	cg->SetDistStrength(m_dist_strength);
	cg->SetDistFilterSize(m_dist_filter_size);
	cg->SetMaxDist(m_max_dist);
	cg->SetDistThresh(m_dist_thresh);
	cg->SetDiffusion(m_diff);
	cg->SetFalloff(m_falloff);
	cg->SetSize(m_size);
	cg->SetSizeLimit(m_size_lm);
	cg->SetDensity(m_density);
	cg->SetDensityThresh(m_density_thresh);
	cg->SetVarThresh(m_varth);
	cg->SetDensityWinSize(m_density_window_size);
	cg->SetDensityStatSize(m_density_stats_size);
	cg->SetFixate(m_fixate);
	cg->SetGrowFixed(m_grow_fixed);
	cg->SetClean(m_clean);
	cg->SetCleanIter(m_clean_iter);
	cg->SetCleanSize(m_clean_size_vl);
	cg->SetFillBorder(m_fill_border);
}

void ComponentDefault::Set(flrd::Clusterizer* cl)
{
	if (!cl)
		return;

	m_cluster_method = cl->GetMethod();
	m_cluster_clnum = cl->GetNum();
	m_cluster_maxiter = cl->GetMaxIter();
	m_cluster_tol = cl->GetTol();
	m_cluster_size = cl->GetSize();
	m_cluster_eps = cl->GetEps();
}

void ComponentDefault::Apply(flrd::Clusterizer* cl)
{
	if (!cl)
		return;

	cl->SetMethod(m_cluster_method);
	cl->SetNum(m_cluster_clnum);
	cl->SetMaxIter(m_cluster_maxiter);
	cl->SetTol(m_cluster_tol);
	cl->SetSize(m_cluster_size);
	cl->SetEps(m_cluster_eps);
}

void ComponentDefault::Set(flrd::ComponentSelector* cs)
{
	if (!cs)
		return;

	m_use_min = cs->GetUseMin();
	m_min_num = cs->GetMinNum();
	m_use_max = cs->GetUseMax();
	m_max_num = cs->GetMaxNum();
}

void ComponentDefault::Apply(flrd::ComponentSelector* cs)
{
	if (!cs)
		return;

	cs->SetUseMin(m_use_min);
	cs->SetUseMax(m_use_max);
	cs->SetMinNum(m_min_num);
	cs->SetMaxNum(m_max_num);
}

void ComponentDefault::Set(flrd::ComponentAnalyzer* ca)
{
	if (!ca)
		return;

	m_slimit = ca->GetSizeLimit();
	m_colocal = ca->GetColocal();
	m_consistent = ca->GetConsistent();
	m_channel_type = ca->GetChannelType();
	m_color_type = ca->GetColorType();
	m_annot_type = ca->GetAnnotType();
	m_use_dist_neighbor = ca->GetUseDistNeighbor();
	m_dist_neighbor_num = ca->GetDistNeighborNum();
	m_use_dist_allchan = ca->GetUseDistAllchan();
}

void ComponentDefault::Apply(flrd::ComponentAnalyzer* ca)
{
	if (!ca)
		return;

	ca->SetSizeLimit(m_slimit);
	ca->SetColocal(m_colocal);
	ca->SetConsistent(m_consistent);
	ca->SetChannelType(m_channel_type);
	ca->SetColorType(m_color_type);
	ca->SetAnnotType(m_annot_type);
	ca->SetUseDistNeighbor(m_use_dist_neighbor);
	ca->SetDistNeighborNum(m_dist_neighbor_num);
	ca->SetUseDistAllchan(m_use_dist_allchan);
}