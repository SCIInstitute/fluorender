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
#ifndef _VOLUMEDEFAULT_H_
#define _VOLUMEDEFAULT_H_

#include <wx/fileconf.h>

class VolumeData;
class VolumeDataDefault
{
public:
	VolumeDataDefault();
	~VolumeDataDefault();

	void Read(wxFileConfig& f);
	void Save(wxFileConfig& f);
	void Set(VolumeData* vd);
	void Apply(VolumeData* vd);

public:
	//default values
	bool m_gamma_enable;
	double m_gamma;

	bool m_boundary_enable;
	double m_boundary;

	bool m_saturation_enable;
	double m_saturation;

	bool m_thresh_enable;
	double m_lo_thresh;
	double m_hi_thresh;
	double m_sw;

	bool m_luminance_enable;
	double m_luminance;

	bool m_alpha_enable;
	double m_alpha;

	//shading
	bool m_shading_enable;
	double m_low_shading;
	double m_high_shading;

	//shadow
	bool m_shadow_enable;
	double m_shadow_intensity;

	bool m_sample_rate_enable;
	double m_sample_rate;

	//spacing
	double m_spcx, m_spcy, m_spcz;

	//color map mode
	int m_colormap_mode;	//0-normal; 1-rainbow
	bool m_colormap_disp;	//true/false
	double m_colormap_low_value;
	double m_colormap_hi_value;
	double m_colormap_inv;
	int m_colormap_type;//index to a colormap
	int m_colormap_proj;//index to a way of projection

	//noise reduction
	bool m_noise_rd;
	//interpolate
	bool m_interpolate;
	//inverted
	bool m_inverted;
	//mip
	bool m_mip_enable;
	//transparent
	bool m_transparent;
	//blend mode
	int m_blend_mode;	//0: ignore; 1: layered; 2: depth; 3: composite
	//shown in legend
	bool m_legend;
	//lable
	int m_label_mode;

};
#endif
