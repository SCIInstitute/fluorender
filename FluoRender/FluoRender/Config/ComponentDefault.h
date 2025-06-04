/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <string>

namespace flrd
{
	class ComponentGenerator;
	class ComponentAnalyzer;
	class Clusterizer;
	class ComponentSelector;
}
class ComponentDefault
{
public:
	ComponentDefault();
	~ComponentDefault();

	void Read(const std::wstring& filename);
	void Save(const std::wstring& filename);
	void Read(const std::string& gst_file);
	void Save(const std::string& gst_file);
	void Set(flrd::ComponentGenerator* cg);
	void Apply(flrd::ComponentGenerator* cg);
	void Set(flrd::Clusterizer* cl);
	void Apply(flrd::Clusterizer* cl);
	void Set(flrd::ComponentSelector* cs);
	void Apply(flrd::ComponentSelector* cs);
	void Set(flrd::ComponentAnalyzer* ca);
	void Apply(flrd::ComponentAnalyzer* ca);

public:
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

	//noise removal settings
	double m_nr_thresh;
	int m_nr_size;
	bool m_nr_preview;
	double m_nr_hdr_r;
	double m_nr_hdr_g;
	double m_nr_hdr_b;

	//cluster settings
	int m_cluster_method;//0:exmax; 1:dbscan; 2:kmeans
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

	//analyzer
	size_t m_slimit;//size limit for connecting components
	bool m_consistent;
	bool m_colocal;
	int m_channel_type;//channel type: 1-multichannel; 2-rgb channel
	int m_color_type;//color_type: 1-id-based; 2-size-based
	int m_annot_type;//annot type: 1-id; 2:serianl number

	//distance
	bool m_use_dist_neighbor;
	int m_dist_neighbor_num;
	bool m_use_dist_allchan;

	//record
	bool m_record_cmd;
};
#endif
