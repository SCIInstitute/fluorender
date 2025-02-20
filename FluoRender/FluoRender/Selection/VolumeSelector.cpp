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

#include <Global.h>
#include <RenderCanvas.h>
#include <MainFrame.h>
#include <compatibility.h>
#include <TextureRenderer.h>
#include <PaintBoxes.h>
#include <MaskBorder.h>
#include <Histogram.h>
#include <RecordHistParams.h>
#include <glm/gtc/type_ptr.hpp>
#include <Debug.h>

using namespace flrd;

VolumeSelector::VolumeSelector() :
	m_vd(0),
	m_vd_copy(0),
	m_copy_data(false),
	m_2d_mask(0),
	m_2d_weight1(0),
	m_2d_weight2(0),
	m_iter_num(20),
	m_mode_ext(0),
	m_mode(2),
	m_init_mask(3),
	m_use2d(false),
	m_update_order(true),
	m_ini_thresh(0.0),
	m_gm_falloff(1.0),
	m_scl_falloff(0.0),
	m_scl_translate(0.0),
	m_select_multi(false),
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
	m_air_press(0.5),
	m_iter(0),
	m_iter_weak(10),
	m_iter_normal(30),
	m_iter_strong(60),
	m_brush_sets_index(0),
	m_test_speed(false)
{
}

VolumeSelector::~VolumeSelector()
{
}

void VolumeSelector::SetMode(int mode)
{
	m_mode_ext = mode;
	if (m_mode_ext)
		m_mode = m_mode_ext;
	ChangeBrushSetsIndex();
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;

	switch (m_mode_ext)
	{
	case 0://not used
		canvas->SetIntMode(1);
		break;
	case 1://select
	case 2://append
	case 3://erase
	case 4://diffuse
	case 5://flood
	case 6://clear
	case 7://select all
	case 8://select solid
		canvas->SetIntMode(2);
		break;
	case 9://grow from point
		canvas->SetIntMode(10);
		break;
	}
}

//segment volumes in current view
void VolumeSelector::Segment(bool push_mask, bool est_th, int mx, int my)
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!m_vd)
		return;

	//add ml record
	if (glbin.get_cg_table_enable() &&
		glbin.get_cg_entry().getValid() &&
		m_vd)
	{
		//histogram
		flrd::Histogram histogram(m_vd);
		histogram.SetUseMask(true);
		flrd::EntryHist* eh = histogram.GetEntryHist();

		if (eh)
		{
			//record
			flrd::RecordHistParams* rec = new flrd::RecordHistParams();
			rec->setInput(eh);
			flrd::EntryParams* ep = new flrd::EntryParams(glbin.get_cg_entry());
			rec->setOutput(ep);

			//table
			glbin.get_cg_table().addRecord(rec);
		}
	}

	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;

	canvas->HandleCamera();
	if (m_mode == 9)
		segment(push_mask, est_th, mx, my);
	else
		segment(push_mask, est_th, 0, 0);
}

void VolumeSelector::segment(bool push_mask, bool est_th, int mx, int my)
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!canvas || !m_vd)
		return;

	if (m_test_speed)
		m_t1 = std::chrono::high_resolution_clock::now();

	//notify volume that mask is cleared
	if (m_mode == 6)
		m_vd->SetMaskClear();

	//save view
	m_mv_mat = canvas->GetDrawMat();
	m_prj_mat = canvas->GetProjection();

	//mouse position
	fluo::Vector mvec;//mouse vector in data space
	bool valid_mvec = false;
	if (m_mode == 9)
	{
		GLint mp[2] = { mx, my };
		m_vd->GetVR()->set_mouse_position(mp);
		valid_mvec = GetMouseVec(mx, my, mvec);
	}
	m_vd->GetVR()->set_mouse_vec(mvec);

	flvr::Framebuffer* paint_buffer =
		glbin_framebuffer_manager.framebuffer("paint brush");
	if (paint_buffer)
		Set2DMask(paint_buffer->tex_id(GL_COLOR_ATTACHMENT0));
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer(
			"final");
	flvr::Framebuffer* chann_buffer =
		glbin_framebuffer_manager.framebuffer(
			"channel");
	if (final_buffer && chann_buffer)
		Set2DWeight(
			final_buffer->tex_id(GL_COLOR_ATTACHMENT0),
			chann_buffer->tex_id(GL_COLOR_ATTACHMENT0));
	//orthographic
	SetOrthographic(!canvas->GetPersp());

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
	if (m_select_multi)
	{
		DataGroup* group = glbin_current.vol_group;
		if (group && group->GetVolumeNum() > 1)
		{
			VolumeData* save = m_vd;
			for (int i = 0; i < group->GetVolumeNum(); i++)
			{
				VolumeData* vd = group->GetVolumeData(i);
				if (vd && vd->GetDisp())
				{
					m_vd = vd;
					Select(push_mask, est_th, r);
				}
			}
			m_vd = save;
		}
		else
			Select(push_mask, est_th, r);
	}
	else
		Select(push_mask, est_th, r);

	//restore
	if (press)
	{
		m_gm_falloff = gm_falloff_save;
		m_scl_translate = scl_translate_save;
	}

	if (m_test_speed)
	{
		m_t2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_span =
			std::chrono::duration_cast<std::chrono::duration<double>>(
				m_t2 - m_t1);
		m_span_sec = time_span.count();
	}
}

void VolumeSelector::Select(bool push_mask, bool est_th, double radius)
{
	RenderCanvas* canvas = glbin_current.canvas;
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!canvas || !m_vd)
		return;

	//insert the mask volume into m_vd
	m_vd->AddEmptyMask(0, false);
	m_vd->Set2dMask(m_2d_mask);
	if (m_use2d && glIsTexture(m_2d_weight1) && glIsTexture(m_2d_weight2))
		m_vd->Set2DWeight(m_2d_weight1, m_2d_weight2);
	else
		m_vd->Set2DWeight(0, 0);

	if (push_mask)
		PushMask();

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
	m_vd->GetTexture()->deact_all_mask();

	//clear selection
	if (m_mode == 1)
	{
		//has to update twice
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
	}
	else if (m_mode == 6)
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);

	//set up paint mask flags
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (m_mode == 1 || m_mode == 2 ||
		m_mode == 3 || m_mode == 4 ||
		m_mode == 8 || m_mode == 9)
	{
		if (bricks->size() > 1)
		{
			flrd::PaintBoxes pb;
			pb.SetBricks(bricks);
			pb.SetPersp(!canvas->GetPersp());
			fluo::Transform *tform = m_vd->GetTexture()->transform();
			double mvmat[16];
			tform->get_trans(mvmat);
			glm::mat4 mv_mat2 = glm::mat4(
				mvmat[0], mvmat[4], mvmat[8], mvmat[12],
				mvmat[1], mvmat[5], mvmat[9], mvmat[13],
				mvmat[2], mvmat[6], mvmat[10], mvmat[14],
				mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
			mv_mat2 = m_mv_mat * mv_mat2;
			glm::mat4 cmat = m_prj_mat * mv_mat2;
			fluo::Transform mv, pr, mat;
			mv.set(glm::value_ptr(mv_mat2));
			pr.set(glm::value_ptr(m_prj_mat));
			mat.set(glm::value_ptr(cmat));
			pb.SetMats(mv, pr, mat);
			pb.SetPaintTex(m_2d_mask, canvas->GetGLSize().w(), canvas->GetGLSize().h());
			if (m_mode == 9)
				pb.SetMousePos(m_mx, m_my);
			pb.Compute();
		}
		else
		{
			m_vd->GetTexture()->act_all_mask();
			m_vd->GetTexture()->valid_all_mask();
		}
	}

	//initialization
	if (m_init_mask & 1)
	{
		int hr_mode = m_hidden_removal ? (m_ortho ? 1 : 2) : 0;
		if ((m_mode == 1 || m_mode == 2) &&
			m_estimate_threshold && est_th)
		{
			m_vd->DrawMask(0, m_mode, hr_mode, 0.0, gm_falloff, scl_falloff, 0.0, m_w2d, 0.0, 0, false, true);
			m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
			ini_thresh = m_vd->GetEstThresh() * m_vd->GetScalarScale();
			if (m_iter_num > m_iter_weak)
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
			if (m_mode == 9)
			{
				if (m_iter_num <= m_iter_weak)
					m_iter = m_iter_num / 3;
				else if (m_iter_num <= m_iter_normal)
					m_iter = m_iter_num / 2;
				else
					m_iter = m_iter_num;
			}
			else
				m_iter = m_iter_num * (radius / 200.0 > 1.0 ? radius / 200.0 : 1.0);
			int div = 3;
			int order;
			flrd::MaskBorder mb(m_vd);
			for (int i = 0; i < m_iter; i++)
			{
				order = m_update_order ? (i%div) : 0;
				if (m_mode == 1 ||
					m_mode == 2 ||
					m_mode == 4 ||
					m_mode == 9)
					mb.Compute(order);
				m_vd->DrawMask(1, m_mode, 0, ini_thresh,
					gm_falloff, scl_falloff,
					m_scl_translate, m_w2d, 0.0,
					order);
			}
		}
	}

	if (flvr::Texture::mask_undo_num_>0 &&
		m_vd->GetVR())
		m_vd->GetVR()->return_mask();

	if (m_mode == 6)
	{
		m_vd->SetUseMaskThreshold(false);
		m_vd->GetTexture()->invalid_all_mask();
	}
}

//erase selection
void VolumeSelector::Clear()
{
	int mode = m_mode;
	m_mode = 6;
	Segment(true);
	m_mode = mode;
}

//extract a new volume excluding the selection
void VolumeSelector::Erase()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	int mode = 6;
	std::wstring vd_name;
	if (m_vd)
		vd_name = m_vd->GetName();
	if (vd_name.find(L"_DELETED") != std::wstring::npos)
		mode = 7;
	std::wstring group_name;
	DataGroup* group = glbin_current.vol_group;
	if (group)
		group_name = group->GetName();
	glbin_vol_calculator.SetVolumeA(m_vd);
	glbin_vol_calculator.CalculateGroup(mode, group_name);
}

//extract a new volume of the selection
void VolumeSelector::Extract()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	std::wstring group_name;
	DataGroup* group = glbin_current.vol_group;
	if (group)
		group_name = group->GetName();
	glbin_vol_calculator.SetVolumeA(m_vd);
	glbin_vol_calculator.CalculateGroup(5, group_name);
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
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!m_vd ||
		!m_vd->GetTexture() ||
		(select&&m_vd->GetTexture()->nmask()==-1) ||
		m_vd->GetTexture()->nlabel()==-1)
		return;

	if (select)
		m_vd->GetVR()->return_mask();

	//get all the data from original volume
	flvr::Texture* tex_mvd = m_vd->GetTexture();
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
	vd_r->SetName(m_vd->GetName() + L"_COMP1");
	vd_r->SetCurChannel(0);
	//green volume
	if (!vd_g)
		vd_g = new VolumeData();
	vd_g->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_vd->GetName() + L"_COMP2");
	vd_g->SetCurChannel(1);
	//blue volume
	if (!vd_b)
		vd_b = new VolumeData();
	vd_b->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(m_vd->GetName() + L"_COMP3");
	vd_b->SetCurChannel(2);

	//get new data
	//red volume
	flvr::Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return;
	Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
	if (!nrrd_vd_r) return;
	unsigned char* data_vd_r = (unsigned char*)nrrd_vd_r->data;
	if (!data_vd_r) return;
	//green volume
	flvr::Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return;
	Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
	if (!nrrd_vd_g) return;
	unsigned char* data_vd_g = (unsigned char*)nrrd_vd_g->data;
	if (!data_vd_g) return;
	//blue volume
	flvr::Texture* tex_vd_b = vd_b->GetTexture();
	if (!tex_vd_b) return;
	Nrrd* nrrd_vd_b = tex_vd_b->get_nrrd(0);
	if (!nrrd_vd_b) return;
	unsigned char* data_vd_b = (unsigned char*)nrrd_vd_b->data;
	if (!data_vd_b) return;

	if (hide)
		m_randv = int((double)std::rand()/(RAND_MAX)*900+100);
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
			fluo::Color color;
			if (hmode == 0)
				color = fluo::Color(value_label, m_vd->GetShuffle());
			else
			{
				double hue = HueCalculation(hmode, value_label);
				color = fluo::Color(fluo::HSVColor(hue, 1.0, 1.0));
			}
			//color
			value = value>1.0?1.0:value;
			data_vd_r[idx] = (unsigned char)(color.r()*255.0*value);
			data_vd_g[idx] = (unsigned char)(color.g()*255.0*value);
			data_vd_b[idx] = (unsigned char)(color.b()*255.0*value);
		}
	}

	glbin_vol_def.Copy(vd_r, m_vd);
	glbin_vol_def.Copy(vd_g, m_vd);
	glbin_vol_def.Copy(vd_b, m_vd);

	fluo::Color red(  1.0,0.0,0.0);
	fluo::Color green(0.0,1.0,0.0);
	fluo::Color blue( 0.0,0.0,1.0);
	vd_r->SetColor(red);
	vd_g->SetColor(green);
	vd_b->SetColor(blue);

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
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas || (m_mode != 1 &&
		m_mode != 2 && m_mode != 4))
		return false;
	glm::mat4 mv_mat = canvas->GetDrawMat();
	glm::mat4 prj_mat = canvas->GetProjection();
	//compare view
	if (mv_mat == m_mv_mat && prj_mat == m_prj_mat)
		return true;
	else
		return false;
}

void VolumeSelector::PushMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!m_vd || !m_vd->GetTexture())
		return;
	if (flvr::Texture::mask_undo_num_ > 0)
		m_vd->GetTexture()->push_mask();
}

void VolumeSelector::PopMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->pop_mask();
	m_vd->GetVR()->clear_tex_mask();
}

void VolumeSelector::UndoMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_backward();
	m_vd->GetVR()->clear_tex_mask();
}

void VolumeSelector::RedoMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_forward();
	m_vd->GetVR()->clear_tex_mask();
}

//mask operations
void VolumeSelector::CopyMask(bool copy_data)
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (m_vd)
	{
		m_vd_copy = m_vd;
		m_copy_data = copy_data;
	}
}

void VolumeSelector::PasteMask(int op)
{
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (m_vd && m_vd_copy)
	{
		//prevent self copying
		if (!m_copy_data &&
			m_vd == m_vd_copy)
			return;

		//undo/redo
		if (flvr::Texture::mask_undo_num_ > 0 &&
			m_vd->GetTexture())
			m_vd->GetTexture()->push_mask();
		if (m_copy_data)
		{
			Nrrd* data = m_vd_copy->GetVolume(false);
			if (m_vd_copy->GetBits() == 16)
				m_vd->AddMask16(data, op, m_vd_copy->GetScalarScale());
			else
				m_vd->AddMask(data, op);
		}
		else
			m_vd->AddMask(m_vd_copy->GetMask(false), op);

		if (m_select_multi)
		{
			DataGroup* group = glbin_current.vol_group;
			Nrrd* data = m_vd->GetMask(false);
			if (group && data)
				group->AddMask(data, 0);
		}
	}
}

bool VolumeSelector::GetMouseVec(int mx, int my, fluo::Vector &mvec)
{
	DBGPRINT(L"mx: %d\tmy: %d\n", mx, my);
	RenderCanvas* canvas = glbin_current.canvas;
	if (!m_vd)
		m_vd = glbin_current.vol_data;
	if (!canvas || !m_vd)
		return false;
	if (mx >= 0 && my >= 0 &&
		m_mx0 >=0 && m_my0 >=0)
	{
		double dist = (m_mx - mx)*(m_mx - mx) + (m_my - my)*(m_my - my);
		if (dist < 5000.0)
		{
			//user can set a direction then stay
			mvec = m_mvec;
			return true;
		}
	}
	m_mx0 = m_mx;
	m_my0 = m_my;
	m_mx = mx;
	m_my = my;
	if (m_mx < 0 || m_my < 0 ||
		m_mx0 < 0 || m_my0 < 0)
		return false;
	
	int nx = canvas->GetGLSize().w();
	int ny = canvas->GetGLSize().h();
	fluo::Transform *tform = m_vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	glm::mat4 mv_mat = canvas->GetDrawMat();
	mv_mat = mv_mat * mv_mat2;
	fluo::Transform mv, pr;
	mv.set(glm::value_ptr(mv_mat));
	pr.set(glm::value_ptr(m_prj_mat));
	mv.invert();
	pr.invert();
	fluo::Vector v;
	fluo::Point p0(double(m_mx0)*2/nx - 1.0, 1.0 - double(m_my0)*2/ny, 0);
	fluo::Point p1(double(m_mx)*2/nx - 1.0, 1.0 - double(m_my)*2/ny, 0);
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