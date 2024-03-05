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
#ifndef _COMPONENTDEFAULT_H_
#define _COMPONENTDEFAULT_H_

#include <wx/fileconf.h>

namespace flrd
{
	class ComponentGenerator;
	class ComponentAnalyzer;
}
class ComponentDefault
{
public:
	ComponentDefault();
	~ComponentDefault();

	void Read(const std::string& filename);
	void Save(const std::string& filename);
	void Read(wxFileConfig& f);
	void Save(wxFileConfig& f);
	void Set(flrd::ComponentGenerator* cg);
	void Apply(flrd::ComponentGenerator* cg);
	//void Set(flrd::ComponentAnalyzer* ca);
	//void Apply(flrd::ComponentAnalyzer* ca);

public:
	//default values
	//bool m_ca_select_only; m_use_sel
	//int m_ca_min;
	//int m_ca_max;
	//bool m_ca_ignore_max; m_use_min && !m_use_max

	//generate settings
	bool m_use_sel;
	bool m_use_ml;
	int m_iter;
	double m_thresh;
	double m_tfactor;
	//distance field
	bool m_use_dist_field;
	double m_dist_strength;
	int m_dist_filter_size;
	int m_max_dist;
	double m_dist_thresh;
	//diffusion
	bool m_diff;
	double m_falloff;
	bool m_size;
	int m_size_lm;
	//density
	bool m_density;
	double m_density_thresh;
	double m_varth;//variance threshold
	int m_density_window_size;
	int m_density_stats_size;
	//fixate
	bool m_fixate;
	int m_fix_size;
	int m_grow_fixed;
	//clean
	bool m_clean;
	int m_clean_iter;
	int m_clean_size_vl;
	//fill borders
	double m_fill_border;

	//cluster settings
	bool m_cluster_method_exmax;
	bool m_cluster_method_dbscan;
	bool m_cluster_method_kmeans;
	//parameters
	int m_cluster_clnum;
	int m_cluster_maxiter;
	float m_cluster_tol;
	int m_cluster_size;
	double m_cluster_eps;

	//selection
	bool m_use_min;
	int m_min_num;
	bool m_use_max;
	int m_max_num;
	//options
	bool m_consistent;
	bool m_colocal;

	//modify
	unsigned int m_cell_new_id;
	bool m_cell_new_id_empty;

	//distance
	bool m_use_dist_neighbor;
	int m_dist_neighbor;
	bool m_use_dist_allchan;

	//output
	int m_output_type;//1-multi; 2-rgb;

	//auto udate
	bool m_auto_update;

	//record
	bool m_record_cmd;
};
#endif
