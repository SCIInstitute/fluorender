/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include "VolumeSelector.h"
#include "DataManager.h"
#include "VRenderGLView.h"
#include "VRenderFrame.h"
#include "utility.h"
#include <FLIVR/Framebuffer.h>
#include <FLIVR/TextureRenderer.h>
#include <wx/wx.h>

VolumeSelector::VolumeSelector() :
	m_view(0),
	m_vd(0),
	m_2d_mask(0),
	m_2d_weight1(0),
	m_2d_weight2(0),
	m_iter_num(20),
	m_mode(0),
	m_use2d(false),
	m_update_order(true),
	m_ini_thresh(0.0),
	m_gm_falloff(1.0),
	m_scl_falloff(0.0),
	m_scl_translate(0.0),
	m_select_multi(0),
	m_use_brush_size2(false),
	m_edge_detect(false),
	m_hidden_removal(false),
	m_ortho(true),
	m_w2d(0.0),
	m_randv(113),
	m_ps(false),
	m_estimate_threshold(false)
{
}

VolumeSelector::~VolumeSelector()
{
}

void VolumeSelector::Segment()
{
	if (!m_view || !m_vd)
		return;

	glm::mat4 mv_mat = m_view->GetDrawMat();

	FLIVR::Framebuffer* paint_buffer =
		FLIVR::TextureRenderer::framebuffer_manager_.framebuffer("paint brush");
	if (paint_buffer)
		Set2DMask(paint_buffer->tex_id(GL_COLOR_ATTACHMENT0));
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
			"final");
	Framebuffer* chann_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
			"channel");
	if (final_buffer && chann_buffer)
		Set2DWeight(
			final_buffer->tex_id(GL_COLOR_ATTACHMENT0),
			chann_buffer->tex_id(GL_COLOR_ATTACHMENT0));
	//orthographic
	SetOrthographic(!m_view->GetPersp());

	//modulate threshold with pressure
	double gm_falloff_save, scl_translate_save;
	double press_peak = m_view->GetBrushPressPeak();
	bool press = m_view->GetBrushUsePres() && press_peak > 0.0;
	if (press)
	{
		//gradient magnitude falloff
		gm_falloff_save = m_gm_falloff;
		m_gm_falloff = gm_falloff_save + press_peak * 0.5;
		//scalar translate
		scl_translate_save = m_scl_translate;
		m_scl_translate = scl_translate_save - press_peak + 0.5;
		if (m_scl_translate < 0.0) m_scl_translate = 0.0;
	}

	double r = m_view->GetBrushSize2() - m_view->GetBrushSize1();
	if (m_select_multi == 1)
	{
		DataGroup* group = m_view->GetGroup(m_vd);
		if (group && group->GetVolumeNum() > 1)
		{
			VolumeData* save = m_vd;
			for (int i = 0; i < group->GetVolumeNum(); i++)
			{
				VolumeData* vd = group->GetVolumeData(i);
				if (vd && vd->GetDisp())
				{
					m_vd = vd;
					Select(r);
				}
			}
			m_vd = save;
		}
		else
			Select(r);
	}
	else
		Select(r);

	//restore
	if (press)
	{
		m_gm_falloff = gm_falloff_save;
		m_scl_translate = scl_translate_save;
	}
}

void VolumeSelector::Select(double radius)
{
	if (!m_vd)
		return;

	//insert the mask volume into m_vd
	m_vd->AddEmptyMask(0, false);
	m_vd->Set2dMask(m_2d_mask);
	if (m_use2d && glIsTexture(m_2d_weight1) && glIsTexture(m_2d_weight2))
		m_vd->Set2DWeight(m_2d_weight1, m_2d_weight2);
	else
		m_vd->Set2DWeight(0, 0);

	if (Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

	//segment the volume with 2d mask
	//result in 3d mask
	//clear if the select mode
	double ini_thresh, gm_falloff, scl_falloff;
	if (m_use_brush_size2)
	{
		if (m_ini_thresh > 0.0)
			ini_thresh = m_ini_thresh;
		else
			ini_thresh = sqrt(m_scl_translate);
		if (m_scl_falloff > 0.0)
			scl_falloff = m_scl_falloff;
		else
			scl_falloff = 0.008;
	}
	else
	{
		ini_thresh = m_scl_translate;
		scl_falloff = 0.0;
	}
	if (m_edge_detect)
		gm_falloff = m_gm_falloff;
	else
		gm_falloff = 1.0;

	//there is some unknown problem of clearing the mask
	if (m_mode == 1)
	{
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
	}
	else if (m_mode == 6)
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);

	//initialization
	int hr_mode = m_hidden_removal?(m_ortho?1:2):0;
	if ((m_mode==1 || m_mode==2) && m_estimate_threshold)
	{
		m_vd->DrawMask(0, m_mode, hr_mode, 0.0, gm_falloff, scl_falloff, 0.0, m_w2d, 0.0, 0, false, true);
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
		ini_thresh = m_vd->GetEstThresh() * m_vd->GetScalarScale();
		if (m_iter_num>BRUSH_TOOL_ITER_WEAK)
			ini_thresh /= 2.0;
		m_scl_translate = ini_thresh;
	}
	m_vd->DrawMask(0, m_mode, hr_mode, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);

	//grow the selection when paint mode is select, append, erase, or invert
	if (m_mode==1 ||
		m_mode==2 ||
		m_mode==3 ||
		m_mode==4)
	{
		//loop for growing
		int iter = m_iter_num*(radius/200.0>1.0?radius/200.0:1.0);
		int div = iter / 3;
		div = div ? div : 1;
		for (int i=0; i<iter; i++)
			m_vd->DrawMask(1, m_mode, 0, ini_thresh,
				gm_falloff, scl_falloff,
				m_scl_translate, m_w2d, 0.0,
				m_update_order?(i%div):0);
	}

	if (m_mode == 6)
		m_vd->SetUseMaskThreshold(false);

	if (Texture::mask_undo_num_>0 &&
		m_vd->GetVR())
		m_vd->GetVR()->return_mask();
}

double VolumeSelector::HueCalculation(int mode, unsigned int label)
{
	double hue = 0.0;
	switch (mode)
	{
	case 0:
		hue = double(label % 360);
		break;
	case 1:
		hue = double(label % m_randv) / double(m_randv) * 360.0;
		break;
	case 2:
		hue = double(bit_reverse(label) % m_randv) / double(m_randv) * 360.0;
		break;
	}
	return hue;
}

void VolumeSelector::CompExportRandomColor(int hmode, VolumeData* vd_r,
	VolumeData* vd_g, VolumeData* vd_b, bool select, bool hide)
{
	if (!m_vd ||
		!m_vd->GetTexture() ||
		(select&&m_vd->GetTexture()->nmask()==-1) ||
		m_vd->GetTexture()->nlabel()==-1)
		return;

	if (select)
		m_vd->GetVR()->return_mask();

	//get all the data from original volume
	Texture* tex_mvd = m_vd->GetTexture();
	if (!tex_mvd) return;
	Nrrd* nrrd_mvd = tex_mvd->get_nrrd(0);
	if (!nrrd_mvd) return;
	Nrrd* nrrd_mvd_mask = tex_mvd->get_nrrd(tex_mvd->nmask());
	if (select && !nrrd_mvd_mask) return;
	Nrrd* nrrd_mvd_label = tex_mvd->get_nrrd(tex_mvd->nlabel());
	if (!nrrd_mvd_label) return;
	void* data_mvd = nrrd_mvd->data;
	unsigned char* data_mvd_mask = select?(unsigned char*)nrrd_mvd_mask->data:0;
	unsigned int* data_mvd_label = (unsigned int*)nrrd_mvd_label->data;
	if (!data_mvd || (select&&!data_mvd_mask) || !data_mvd_label) return;

	//create new volumes
	int res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;
	int bits = 8;
	m_vd->GetResolution(res_x, res_y, res_z);
	m_vd->GetSpacings(spc_x, spc_y, spc_z);
	int brick_size = m_vd->GetTexture()->get_build_max_tex_size();
	unsigned long long for_size = (unsigned long long)(res_x)*res_y*res_z;

	bool push_new = true;
	//red volume
	if (!vd_r)
		vd_r = new VolumeData();
	else
		push_new = false;
	vd_r->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(m_vd->GetName() +
		wxString::Format("_COMP1"));
	//vd_r->SetCurChannel(0);
	//green volume
	if (!vd_g)
		vd_g = new VolumeData();
	vd_g->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_vd->GetName() +
		wxString::Format("_COMP2"));
	//vd_g->SetCurChannel(1);
	//blue volume
	if (!vd_b)
		vd_b = new VolumeData();
	vd_b->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(m_vd->GetName() +
		wxString::Format("_COMP3"));
	//vd_b->SetCurChannel(2);

	//get new data
	//red volume
	Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return;
	Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
	if (!nrrd_vd_r) return;
	unsigned char* data_vd_r = (unsigned char*)nrrd_vd_r->data;
	if (!data_vd_r) return;
	//green volume
	Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return;
	Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
	if (!nrrd_vd_g) return;
	unsigned char* data_vd_g = (unsigned char*)nrrd_vd_g->data;
	if (!data_vd_g) return;
	//blue volume
	Texture* tex_vd_b = vd_b->GetTexture();
	if (!tex_vd_b) return;
	Nrrd* nrrd_vd_b = tex_vd_b->get_nrrd(0);
	if (!nrrd_vd_b) return;
	unsigned char* data_vd_b = (unsigned char*)nrrd_vd_b->data;
	if (!data_vd_b) return;

	if (hide)
		m_randv = int((double)rand()/(RAND_MAX)*900+100);
	//populate the data
	unsigned long long idx;
	for (idx = 0; idx < for_size; ++idx)
	{
		unsigned int value_label = data_mvd_label[idx];
		if (value_label > 0)
		{
			//intensity value
			double value = 0.0;
			if (nrrd_mvd->type == nrrdTypeUChar)
			{
				if (select)
					value = double(((unsigned char*)data_mvd)[idx]) *
					double(data_mvd_mask[idx]) / 65025.0;
				else
					value = double(((unsigned char*)data_mvd)[idx]) / 255.0;
			}
			else if (nrrd_mvd->type == nrrdTypeUShort)
			{
				if (select)
					value = double(((unsigned short*)data_mvd)[idx]) *
					m_vd->GetScalarScale() *
					double(data_mvd_mask[idx]) / 16581375.0;
				else
					value = double(((unsigned short*)data_mvd)[idx]) *
					m_vd->GetScalarScale() / 65535.0;
			}
			Color color;
			if (hmode == 0)
				color = Color(value_label, m_vd->GetShuffle());
			else
			{
				double hue = HueCalculation(hmode, value_label);
				color = Color(HSVColor(hue, 1.0, 1.0));
			}
			//color
			value = value>1.0?1.0:value;
			data_vd_r[idx] = (unsigned char)(color.r()*255.0*value);
			data_vd_g[idx] = (unsigned char)(color.g()*255.0*value);
			data_vd_b[idx] = (unsigned char)(color.b()*255.0*value);
		}
	}

	FLIVR::Color red    = Color(1.0,0.0,0.0);
	FLIVR::Color green  = Color(0.0,1.0,0.0);
	FLIVR::Color blue   = Color(0.0,0.0,1.0);
	vd_r->SetColor(red);
	vd_g->SetColor(green);
	vd_b->SetColor(blue);
	bool bval = m_vd->GetEnableAlpha();
	vd_r->SetEnableAlpha(bval);
	vd_g->SetEnableAlpha(bval);
	vd_b->SetEnableAlpha(bval);
	bval = m_vd->GetShading();
	vd_r->SetShading(bval);
	vd_g->SetShading(bval);
	vd_b->SetShading(bval);
	vd_r->SetShadow(false);
	vd_g->SetShadow(false);
	vd_b->SetShadow(false);
	//other settings
	double amb, diff, spec, shine;
	m_vd->GetMaterial(amb, diff, spec, shine);
	vd_r->Set3DGamma(m_vd->Get3DGamma());
	vd_r->SetBoundary(m_vd->GetBoundary());
	vd_r->SetOffset(m_vd->GetOffset());
	vd_r->SetLeftThresh(m_vd->GetLeftThresh());
	vd_r->SetRightThresh(m_vd->GetRightThresh());
	vd_r->SetAlpha(m_vd->GetAlpha());
	vd_r->SetSampleRate(m_vd->GetSampleRate());
	vd_r->SetMaterial(amb, diff, spec, shine);
	vd_g->Set3DGamma(m_vd->Get3DGamma());
	vd_g->SetBoundary(m_vd->GetBoundary());
	vd_g->SetOffset(m_vd->GetOffset());
	vd_g->SetLeftThresh(m_vd->GetLeftThresh());
	vd_g->SetRightThresh(m_vd->GetRightThresh());
	vd_g->SetAlpha(m_vd->GetAlpha());
	vd_g->SetSampleRate(m_vd->GetSampleRate());
	vd_g->SetMaterial(amb, diff, spec, shine);
	vd_b->Set3DGamma(m_vd->Get3DGamma());
	vd_b->SetBoundary(m_vd->GetBoundary());
	vd_b->SetOffset(m_vd->GetOffset());
	vd_b->SetLeftThresh(m_vd->GetLeftThresh());
	vd_b->SetRightThresh(m_vd->GetRightThresh());
	vd_b->SetAlpha(m_vd->GetAlpha());
	vd_b->SetSampleRate(m_vd->GetSampleRate());
	vd_b->SetMaterial(amb, diff, spec, shine);

	if (push_new)
	{
		m_result_vols.push_back(vd_r);
		m_result_vols.push_back(vd_g);
		m_result_vols.push_back(vd_b);
	}

	//turn off m_vd
	if (hide)
		m_vd->SetDisp(false);
}

VolumeData* VolumeSelector::GetResult(bool pop)
{
	VolumeData* vd = 0;
	if (!m_result_vols.empty())
	{
		vd = m_result_vols.back();
		if (pop)
			m_result_vols.pop_back();
	}
	return vd;
}

//process current selection
int VolumeSelector::ProcessSel(double thresh)
{
	m_ps = false;

	if (!m_vd ||
		!m_vd->GetTexture() ||
		m_vd->GetTexture()->nmask()==-1)
		return 0;

	m_vd->GetVR()->return_mask();

	//get all the data from original volume
	Texture* tex_mvd = m_vd->GetTexture();
	if (!tex_mvd) return 0;
	Nrrd* nrrd_mvd = tex_mvd->get_nrrd(0);
	if (!nrrd_mvd) return 0;
	Nrrd* nrrd_mvd_mask = tex_mvd->get_nrrd(tex_mvd->nmask());
	if (!nrrd_mvd_mask) return 0;
	void* data_mvd = nrrd_mvd->data;
	unsigned char* data_mvd_mask = (unsigned char*)nrrd_mvd_mask->data;
	if (!data_mvd || (!data_mvd_mask)) return 0;

	//find center
	int res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;
	m_vd->GetResolution(res_x, res_y, res_z);
	m_vd->GetSpacings(spc_x, spc_y, spc_z);

	m_ps_size = 0.0;
	double nw = 0.0;
	double w;
	Point sump(0.0, 0.0, 0.0);
	double value;
	int ii, jj, kk;
	int index;
	for (ii=0; ii<res_x; ii++)
		for (jj=0; jj<res_y; jj++)
			for (kk=0; kk<res_z; kk++)
			{
				index = res_x*res_y*kk + res_x*jj + ii;
				if (nrrd_mvd->type == nrrdTypeUChar)
					value = double(((unsigned char*)data_mvd)[index]) *
					double(data_mvd_mask[index]) / 65025.0;
				else if (nrrd_mvd->type == nrrdTypeUShort)
					value = double(((unsigned short*)data_mvd)[index]) *
					m_vd->GetScalarScale() *
					double(data_mvd_mask[index]) / 16581375.0;
				if (value > thresh)
				{
					w = value>0.5?1.0:-16.0*value*value*value + 12.0*value*value;
					sump += Point(ii, jj, kk)*w;
					nw += w;
					m_ps_size += 1.0;
				}
			}

			//clear data_mvd_mask
			size_t set_num = res_x*res_y*res_z;
			memset(data_mvd_mask, 0, set_num);

			if (nw > 0.0)
			{
				m_ps_center = Point(sump.x()*spc_x, sump.y()*spc_y, sump.z()*spc_z) / nw +
					Vector(0.5*spc_x, 0.5*spc_y, 0.5*spc_z);
				m_ps_size *= spc_x*spc_y*spc_z;
				m_ps = true;
				return 1;
			}
			else
				return 0;
}

//get center
int VolumeSelector::GetCenter(FLIVR::Point& p)
{
	p = m_ps_center;
	return m_ps;
}

//get volume
int VolumeSelector::GetSize(double &s)
{
	s = m_ps_size;
	return m_ps;
}

