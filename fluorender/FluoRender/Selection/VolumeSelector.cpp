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
#include <Selection/PaintBoxes.h>
#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <glm/gtc/type_ptr.hpp>

using namespace FL;

VolumeSelector::VolumeSelector() :
	m_view(0),
	m_vd(0),
	m_2d_mask(0),
	m_2d_weight1(0),
	m_2d_weight2(0),
	m_iter_num(20),
	m_mode(0),
	m_init_mask(3),
	m_use2d(false),
	m_update_order(true),
	m_ini_thresh(0.0),
	m_gm_falloff(1.0),
	m_scl_falloff(0.0),
	m_scl_translate(0.0),
	m_select_multi(0),
	m_edge_detect(false),
	m_hidden_removal(false),
	m_ortho(true),
	m_w2d(0.0),
	m_randv(113),
	m_estimate_threshold(false),
	//paint brush presssure
	m_use_press(true),
	m_on_press(false),
	//paint stroke radius
	m_brush_radius1(10),
	m_brush_radius2(30),
	m_use_brush_radius2(true),
	//paint stroke spacing
	m_brush_spacing(0.1),
	//brush size relation
	m_brush_size_data(true),
	m_pressure(0.0),
	m_press_peak(0.0),
	m_air_press(0.5)
{
}

VolumeSelector::~VolumeSelector()
{
}

void VolumeSelector::Segment(int mx, int my)
{
	if (!m_view || !m_vd)
		return;

	//save view
	m_mv_mat = m_view->GetDrawMat();
	m_prj_mat = m_view->GetProjection();

	//mouse position
	FLIVR::Vector mvec;//mouse vector in data space
	bool valid_mvec = false;
	if (m_mode == 9)
	{
		GLint mp[2] = { mx, my };
		m_vd->GetVR()->set_mouse_position(mp);
		valid_mvec = GetMouseVec(mx, my, mvec);
		//DBGPRINT(L"m_mode:%d\tm_mx:%d\tm_my:%d\tm_mx0:%d\tm_my0:%d\tvalid:%d\n",
		//	m_mode, m_mx, m_my, m_mx0, m_my0, valid_mvec);
		m_vd->GetVR()->set_mouse_vec(mvec);
	}
	else
		m_vd->GetVR()->set_mouse_vec(FLIVR::Vector());

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
	bool press = m_use_press && m_press_peak > 0.0;
	if (press)
	{
		//gradient magnitude falloff
		gm_falloff_save = m_gm_falloff;
		m_gm_falloff = gm_falloff_save + m_press_peak * 0.5;
		//scalar translate
		scl_translate_save = m_scl_translate;
		m_scl_translate = scl_translate_save - m_press_peak + 0.5;
		if (m_scl_translate < 0.0) m_scl_translate = 0.0;
	}

	double r = m_brush_radius2 - m_brush_radius1;
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
	if (m_use_brush_radius2)
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

	//clear paint mask flags
	m_vd->GetTexture()->enable_paint_mask();
	if (m_mode == 1)
	{
		//has to update twice
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
	}
	else if (m_mode == 6)
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);

	//set up paint mask flags
	std::vector<FLIVR::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (bricks->size() > 1 && (
		m_mode == 1 || m_mode == 2 ||
		m_mode == 3 || m_mode == 4 ||
		m_mode == 8 || m_mode == 9))
	{
		FL::PaintBoxes pb;
		pb.SetBricks(bricks);
		pb.SetPersp(!m_view->GetPersp());
		Transform *tform = m_vd->GetTexture()->transform();
		double mvmat[16];
		tform->get_trans(mvmat);
		glm::mat4 mv_mat2 = glm::mat4(
			mvmat[0], mvmat[4], mvmat[8], mvmat[12],
			mvmat[1], mvmat[5], mvmat[9], mvmat[13],
			mvmat[2], mvmat[6], mvmat[10], mvmat[14],
			mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
		mv_mat2 = m_mv_mat * mv_mat2;
		glm::mat4 cmat = m_prj_mat * mv_mat2;
		Transform mv, pr, mat;
		mv.set(glm::value_ptr(mv_mat2));
		pr.set(glm::value_ptr(m_prj_mat));
		mat.set(glm::value_ptr(cmat));
		pb.SetMats(mv, pr, mat);
		if (m_mode == 9)
			pb.SetViewOnly(true);
		else
			pb.SetPaintTex(m_2d_mask, m_view->GetGLSize().x, m_view->GetGLSize().y);
		pb.Compute();
	}

	//initialization
	if (m_init_mask & 1)
	{
		int hr_mode = m_hidden_removal ? (m_ortho ? 1 : 2) : 0;
		if ((m_mode == 1 || m_mode == 2) && m_estimate_threshold)
		{
			m_vd->DrawMask(0, m_mode, hr_mode, 0.0, gm_falloff, scl_falloff, 0.0, m_w2d, 0.0, 0, false, true);
			m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
			ini_thresh = m_vd->GetEstThresh() * m_vd->GetScalarScale();
			if (m_iter_num > BRUSH_TOOL_ITER_WEAK)
				ini_thresh /= 2.0;
			m_scl_translate = ini_thresh;
		}
		m_vd->DrawMask(0, m_mode, hr_mode, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
	}

	//grow the selection when paint mode is select, append, erase, or invert
	if (m_init_mask & 2)
	{
		if (m_mode == 1 ||
			m_mode == 2 ||
			m_mode == 3 ||
			m_mode == 4 ||
			m_mode == 9)
		{
			//loop for growing
			int iter, div;
			if (m_mode == 9)
				iter = m_iter_num / 2;
			else
				iter = m_iter_num * (radius / 200.0 > 1.0 ? radius / 200.0 : 1.0);
			div = iter / 3;
			div = div ? div : 1;
			for (int i = 0; i < iter; i++)
				m_vd->DrawMask(1, m_mode, 0, ini_thresh,
					gm_falloff, scl_falloff,
					m_scl_translate, m_w2d, 0.0,
					m_update_order ? (i%div) : 0);
		}
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

//brush sets
void VolumeSelector::ChangeBrushSetsIndex()
{
	int mode = m_mode;
	if (mode == 1)
		mode = 2;
	for (int i = 0; i < m_brush_radius_sets.size(); ++i)
	{
		BrushRadiusSet radius_set = m_brush_radius_sets[i];
		if (radius_set.type == mode &&
			m_brush_sets_index != i)
		{
			//save previous
			m_brush_radius_sets[m_brush_sets_index].radius1 = m_brush_radius1;
			m_brush_radius_sets[m_brush_sets_index].radius2 = m_brush_radius2;
			m_brush_radius_sets[m_brush_sets_index].use_radius2 = m_use_brush_radius2;
			//get new
			m_brush_radius1 = radius_set.radius1;
			m_brush_radius2 = radius_set.radius2;
			m_use_brush_radius2 = radius_set.use_radius2;
			m_brush_sets_index = i;
			break;
		}
	}
}

//th udpate
bool VolumeSelector::GetThUpdate()
{
	if (!m_view || (m_mode != 1 &&
		m_mode != 2 && m_mode != 4))
		return false;
	glm::mat4 mv_mat = m_view->GetDrawMat();
	glm::mat4 prj_mat = m_view->GetProjection();
	//compare view
	if (mv_mat == m_mv_mat && prj_mat == m_prj_mat)
		return true;
	else
		return false;
}

//load settings
void VolumeSelector::LoadBrushSettings()
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + "/default_brush_settings.dft";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	double val;
	int ival;
	bool bval;

	//brush properties
	if (fconfig.Read("brush_ini_thresh", &val))
		m_ini_thresh = val;
	if (fconfig.Read("brush_gm_falloff", &val))
		m_gm_falloff = val;
	if (fconfig.Read("brush_scl_falloff", &val))
		m_scl_falloff = val;
	if (fconfig.Read("brush_scl_translate", &val))
		m_scl_translate = val;
	//	m_calculator.SetThreshold(val);
	//auto thresh
	if (fconfig.Read("auto_thresh", &bval))
		m_estimate_threshold = bval;
	//edge detect
	if (fconfig.Read("edge_detect", &bval))
		m_edge_detect = bval;
	//hidden removal
	if (fconfig.Read("hidden_removal", &bval))
		m_hidden_removal = bval;
	//select group
	if (fconfig.Read("select_group", &bval))
		m_select_multi = bval ? 1 : 0;
	//brick accuracy
	if (fconfig.Read("accurate_bricks", &bval))
		m_update_order = bval;
	//2d influence
	if (fconfig.Read("brush_2dinfl", &val))
		m_w2d = val;
	//size 1
	if (fconfig.Read("brush_size1", &val) && val > 0.0)
		m_brush_radius1 = val;
	//size 2 link
	if (fconfig.Read("use_brush_size2", &bval))
		m_use_brush_radius2 = bval;
	//size 2
	if (fconfig.Read("brush_size2", &val) && val > 0.0)
		m_brush_radius2 = val;
	//radius settings for individual brush types
	if (fconfig.Exists("/radius_settings"))
	{
		fconfig.SetPath("/radius_settings");
		int brush_num = fconfig.Read("num", 0l);
		if (m_brush_radius_sets.size() != brush_num)
			m_brush_radius_sets.resize(brush_num);
		wxString str;
		for (int i = 0; i < brush_num; ++i)
		{
			str = wxString::Format("/radius_settings/%d", i);
			if (!fconfig.Exists(str))
				continue;
			fconfig.SetPath(str);
			//type
			fconfig.Read("type", &(m_brush_radius_sets[i].type));
			//radius 1
			fconfig.Read("radius1", &(m_brush_radius_sets[i].radius1));
			//radius 2
			fconfig.Read("radius2", &(m_brush_radius_sets[i].radius2));
			//use radius 2
			fconfig.Read("use_radius2", &(m_brush_radius_sets[i].use_radius2));
		}
		fconfig.SetPath("/");
	}
	if (m_brush_radius_sets.size() == 0)
	{
		BrushRadiusSet radius_set;
		//select brush
		radius_set.type = 2;
		radius_set.radius1 = 10;
		radius_set.radius2 = 30;
		radius_set.use_radius2 = true;
		m_brush_radius_sets.push_back(radius_set);
		//erase
		radius_set.type = 3;
		m_brush_radius_sets.push_back(radius_set);
		//diffuse brush
		radius_set.type = 4;
		m_brush_radius_sets.push_back(radius_set);
		//solid brush
		radius_set.type = 8;
		radius_set.use_radius2 = false;
		m_brush_radius_sets.push_back(radius_set);
	}
	m_brush_sets_index = 0;
	//iterations
	if (fconfig.Read("brush_iters", &ival))
	{
		switch (ival)
		{
		case 1:
			m_iter_num = BRUSH_TOOL_ITER_WEAK;
			break;
		case 2:
			m_iter_num = BRUSH_TOOL_ITER_NORMAL;
			break;
		case 3:
			m_iter_num = BRUSH_TOOL_ITER_STRONG;
			break;
		}
	}
	//brush size relation
	if (fconfig.Read("brush_size_data", &bval))
		m_brush_size_data = bval;
}

void VolumeSelector::SaveBrushSettings()
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_brush_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);
	wxString str;
	//brush properties
	fconfig.Write("brush_ini_thresh", m_ini_thresh);
	fconfig.Write("brush_gm_falloff", m_gm_falloff);
	fconfig.Write("brush_scl_falloff", m_scl_falloff);
	fconfig.Write("brush_scl_translate", m_scl_translate);
	//auto thresh
	fconfig.Write("auto_thresh", m_estimate_threshold);
	//edge detect
	fconfig.Write("edge_detect", m_edge_detect);
	//hidden removal
	fconfig.Write("hidden_removal", m_hidden_removal);
	//select group
	fconfig.Write("select_group", m_select_multi == 1);
	//brick acccuracy
	fconfig.Write("accurate_bricks", m_update_order);
	//2d influence
	fconfig.Write("brush_2dinfl", m_w2d);
	//size 1
	fconfig.Write("brush_size1", m_brush_radius1);
	//size2 link
	fconfig.Write("use_brush_size2", m_use_brush_radius2);
	//size 2
	fconfig.Write("brush_size2", m_brush_radius2);
	//radius settings for individual brush types
	fconfig.SetPath("/radius_settings");
	int brush_num = m_brush_radius_sets.size();
	fconfig.Write("num", brush_num);
	for (int i = 0; i < brush_num; ++i)
	{
		BrushRadiusSet radius_set = m_brush_radius_sets[i];
		str = wxString::Format("/radius_settings/%d", i);
		fconfig.SetPath(str);
		//type
		fconfig.Write("type", radius_set.type);
		//radius 1
		fconfig.Write("radius1", radius_set.radius1);
		//radius 2
		fconfig.Write("radius2", radius_set.radius2);
		//use radius 2
		fconfig.Write("use_radius2", radius_set.use_radius2);
	}
	fconfig.SetPath("/");
	//iterations
	fconfig.Write("brush_iters", m_iter_num);
	//brush size relation
	fconfig.Write("brush_size_data", m_brush_size_data);

	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + "/default_brush_settings.dft";
	wxFileOutputStream os(dft);
	fconfig.Save(os);
}

void VolumeSelector::PopMask()
{
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->pop_mask();
	m_vd->GetVR()->clear_tex_mask();
}

void VolumeSelector::UndoMask()
{
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_backward();
	m_vd->GetVR()->clear_tex_mask();
}

void VolumeSelector::RedoMask()
{
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_forward();
	m_vd->GetVR()->clear_tex_mask();
}

bool VolumeSelector::GetMouseVec(int mx, int my, FLIVR::Vector &mvec)
{
	if (!m_view || !m_vd)
		return false;
	if (mx >= 0 && my >= 0 &&
		m_mx0 >=0 && m_my0 >=0 &&
		mx == m_mx && my == m_my)
	{
		//user can set a direction then stay
		mvec = m_mvec;
		return true;
	}
	m_mx0 = m_mx;
	m_my0 = m_my;
	m_mx = mx;
	m_my = my;
	if (m_mx < 0 || m_my < 0 ||
		m_mx0 < 0 || m_my0 < 0)
		return false;
	
	int nx = m_view->GetGLSize().x;
	int ny = m_view->GetGLSize().y;
	Transform *tform = m_vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	glm::mat4 mv_mat = m_view->GetDrawMat();
	mv_mat = mv_mat * mv_mat2;
	Transform mv, pr;
	mv.set(glm::value_ptr(mv_mat));
	pr.set(glm::value_ptr(m_prj_mat));
	mv.invert();
	pr.invert();
	FLIVR::Vector v;
	FLIVR::Point p0(double(m_mx0)*2/nx - 1.0, 1.0 - double(m_my0)*2/ny, 0);
	FLIVR::Point p1(double(m_mx)*2/nx - 1.0, 1.0 - double(m_my)*2/ny, 0);
	//transform
	p0 = pr.transform(p0);
	p0 = mv.transform(p0);
	p1 = pr.transform(p1);
	p1 = mv.transform(p1);
	mvec = p0 - p1;
	mvec.normalize();
	m_mvec = mvec;

	return true;
}