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
#include <VolumeSelector.h>
#include <Global.h>
#include <Names.h>
#include <VolumeDefault.h>
#include <AutomateDefault.h>
#include <RenderView.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <MainFrame.h>
#include <compatibility.h>
#include <TextureRenderer.h>
#include <CompGenerator.h>
#include <BaseConvVolMesh.h>
#include <PaintBoxes.h>
#include <MaskBorder.h>
#include <Histogram.h>
#include <RecordHistParams.h>
#include <TableHistParams.h>
#include <VolumeRenderer.h>
#include <Framebuffer.h>
#include <Texture.h>
#include <VolumeCalculator.h>
#include <glm/gtc/type_ptr.hpp>

using namespace flrd;

VolumeSelector::VolumeSelector() :
	m_vd(0),
	m_vd_copy(0),
	m_copy_data(false),
	m_2d_mask(0),
	m_2d_weight1(0),
	m_2d_weight2(0),
	m_iter_num(20),
	m_mode(SelectMode::None),
	m_init_mask(3),
	m_use2d(false),
	m_update_order(true),
	m_ini_thresh(0.0),
	m_gm_falloff(1.0),
	m_scl_falloff(0.0),
	m_scl_translate(0.0),
	m_select_multi(false),
	m_edge_detect(false),
	m_hidden_removal(true),
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
	m_brush_sets_index(0),
	m_test_speed(false)
{
}

VolumeSelector::~VolumeSelector()
{
}

bool VolumeSelector::GetAutoPaintSize()
{
	if (!m_vd)
		return false;
	int get_paint_size = glbin_automate_def.m_paint_size;
	if (get_paint_size == 0)
		return false;
	else if (get_paint_size == 1)
		return true;
	else if (get_paint_size == 2)
	{
		if (m_vd->GetAllBrickNum() > 1)
			return false;
	}
	return true;
}

void VolumeSelector::SetSelectMode(SelectMode mode)
{
	m_mode = mode;
	ChangeBrushSetsIndex();
}

//segment volumes in current view
void VolumeSelector::Segment(bool push_mask, bool est_th, int mx, int my)
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd)
		return;

	//add ml record
	if (glbin.get_cg_table_enable() &&
		glbin.get_cg_entry().getValid() &&
		m_vd)
	{
		//histogram
		flrd::Histogram histogram(m_vd.get());
		histogram.SetProgressFunc(glbin_data_manager.GetProgressFunc());
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

	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	view->HandleCamera();
	if (m_mode == SelectMode::Grow)
		segment(push_mask, est_th, mx, my);
	else
		segment(push_mask, est_th, 0, 0);

	m_vd->ResetMaskCount();

	if (m_mode == SelectMode::Segment &&
		!glbin_comp_generator.IsBusy())
	{
		//generate components
		bool bval = glbin_comp_generator.GetUseSel();
		glbin_comp_generator.SetUseSel(true);
		glbin_comp_generator.GenerateComp();
		glbin_comp_generator.SetUseSel(bval);
	}

	if (m_mode == SelectMode::Mesh &&
		!glbin_conv_vol_mesh->IsBusy())
	{
		//generate mesh
		glbin_conv_vol_mesh->SetVolumeData(m_vd);
		glbin_conv_vol_mesh->SetUseMask(true);
		glbin_conv_vol_mesh->Update(true);
	}
}

void VolumeSelector::segment(bool push_mask, bool est_th, int mx, int my)
{
	auto view = glbin_current.render_view.lock();
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!view || !m_vd)
		return;

	if (m_test_speed)
		m_t1 = std::chrono::high_resolution_clock::now();

	//notify volume that mask is cleared
	if (m_mode == SelectMode::Clear)
		m_vd->SetMaskClear();

	//save view
	m_mv_mat = view->GetDrawMat();
	m_prj_mat = view->GetProjection();

	//mouse position
	fluo::Vector mvec;//mouse vector in data space
	bool valid_mvec = false;
	if (m_mode == SelectMode::Grow)
	{
		GLint mp[2] = { mx, my };
		m_vd->GetVR()->set_mouse_position(mp);
		valid_mvec = GetMouseVec(mx, my, mvec);
	}
	m_vd->GetVR()->set_mouse_vec(mvec);

	auto paint_buffer = glbin_framebuffer_manager.framebuffer(gstRBPaintBrush);
	assert(paint_buffer);
	Set2DMask(paint_buffer->tex_id(flvr::AttachmentPoint::Color(0)));
	auto final_buffer = glbin_framebuffer_manager.framebuffer(gstRBViewFinal);
	auto chann_buffer = glbin_framebuffer_manager.framebuffer(gstRBChannel);
	assert(final_buffer && chann_buffer);
	Set2DWeight(
		final_buffer->tex_id(flvr::AttachmentPoint::Color(0)),
		chann_buffer->tex_id(flvr::AttachmentPoint::Color(0)));
	//orthographic
	SetOrthographic(!view->GetPersp());

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
		auto group = glbin_current.vol_group.lock();
		if (group && group->GetVolumeNum() > 1)
		{
			auto save = m_vd;
			for (int i = 0; i < group->GetVolumeNum(); i++)
			{
				auto vd = group->GetVolumeData(i);
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
	auto view = glbin_current.render_view.lock();
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!view || !m_vd)
		return;

	//insert the mask volume into m_vd
	m_vd->AddEmptyMask(0, false);
	m_vd->Set2dMask(m_2d_mask);
	if (m_use2d && m_2d_weight1 && m_2d_weight2)
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
	if (m_mode == SelectMode::SingleSelect)
	{
		//has to update twice
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
	}
	else if (m_mode == SelectMode::Clear)
		m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);

	//set up paint mask flags
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (m_mode == SelectMode::SingleSelect ||
		m_mode == SelectMode::Append ||
		m_mode == SelectMode::Eraser ||
		m_mode == SelectMode::Diffuse ||
		m_mode == SelectMode::Solid ||
		m_mode == SelectMode::Grow ||
		m_mode == SelectMode::Segment ||
		m_mode == SelectMode::Mesh)
	{
		if (bricks->size() > 1)
		{
			flrd::PaintBoxes pb;
			pb.SetBricks(bricks);
			pb.SetPersp(!view->GetPersp());
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
			pb.SetPaintTex(m_2d_mask, view->GetCanvasSize().w(), view->GetCanvasSize().h());
			if (m_mode == SelectMode::Grow)
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
		if ((m_mode == SelectMode::SingleSelect ||
			m_mode == SelectMode::Append ||
			m_mode == SelectMode::Segment ||
			m_mode == SelectMode::Mesh) &&
			m_estimate_threshold && est_th)
		{
			m_vd->DrawMask(0, static_cast<int>(m_mode), hr_mode, 0.0, gm_falloff, scl_falloff, 0.0, m_w2d, 0.0, 0, false, true);
			m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
			//ini_thresh = m_vd->GetEstThresh() * m_vd->GetScalarScale();
			//if (m_iter_num > 10)
			//	ini_thresh /= 2.0;
			//m_scl_translate = ini_thresh;
		}
		m_vd->DrawMask(0, static_cast<int>(m_mode), hr_mode, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0, 0);
	}

	//grow the selection when paint mode is select, append, erase, or invert
	if (m_init_mask & 2)
	{
		if (m_mode == SelectMode::SingleSelect ||
			m_mode == SelectMode::Append ||
			m_mode == SelectMode::Eraser ||
			m_mode == SelectMode::Diffuse ||
			m_mode == SelectMode::Grow ||
			m_mode == SelectMode::Segment ||
			m_mode == SelectMode::Mesh)
		{
			//loop for growing
			m_iter = m_iter_num * (radius / 200.0 > 1.0 ? radius / 200.0 : 1.0);
			int div = 3;
			int order;
			flrd::MaskBorder mb(m_vd.get());
			for (int i = 0; i < m_iter; i++)
			{
				order = m_update_order ? (i%div) : 0;
				if (m_mode == SelectMode::SingleSelect ||
					m_mode == SelectMode::Append ||
					m_mode == SelectMode::Diffuse ||
					m_mode == SelectMode::Grow ||
					m_mode == SelectMode::Segment ||
					m_mode == SelectMode::Mesh)
					mb.Compute(order);
				m_vd->DrawMask(1, static_cast<int>(m_mode), 0, ini_thresh,
					gm_falloff, scl_falloff,
					m_scl_translate, m_w2d, 0.0,
					order);
			}
		}
	}

	if (flvr::Texture::mask_undo_num_>0 &&
		m_vd->GetVR())
		m_vd->GetVR()->return_mask();

	if (m_mode == SelectMode::Clear)
	{
		m_vd->SetUseMaskThreshold(false);
		m_vd->GetTexture()->invalid_all_mask();
	}
}

//erase selection
void VolumeSelector::Clear()
{
	SelectMode mode = m_mode;
	m_mode = SelectMode::Clear;
	Segment(true);
	m_mode = mode;
}

//extract a new volume excluding the selection
void VolumeSelector::Erase()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	int mode = 6;
	std::wstring vd_name;
	if (m_vd)
		vd_name = m_vd->GetName();
	if (vd_name.find(L"_DELETED") != std::wstring::npos)
		mode = 7;
	std::wstring group_name;
	auto group = glbin_current.vol_group.lock();
	if (group)
		group_name = group->GetName();
	glbin_vol_calculator.SetVolumeA(m_vd);
	glbin_vol_calculator.CalculateGroup(mode, group_name);
}

//extract a new volume of the selection
void VolumeSelector::Extract()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	std::wstring group_name;
	auto group = glbin_current.vol_group.lock();
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

void VolumeSelector::CompExportRandomColor(int hmode,
	std::shared_ptr<VolumeData>& vd_r,
	std::shared_ptr<VolumeData>& vd_g,
	std::shared_ptr<VolumeData>& vd_b,
	bool select, bool hide)
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
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
		vd_r = std::make_shared<VolumeData>();
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
		vd_g = std::make_shared<VolumeData>();
	vd_g->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_vd->GetName() + L"_COMP2");
	vd_g->SetCurChannel(1);
	//blue volume
	if (!vd_b)
		vd_b = std::make_shared<VolumeData>();
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

	glbin_vol_def.Copy(vd_r.get(), m_vd.get());
	glbin_vol_def.Copy(vd_g.get(), m_vd.get());
	glbin_vol_def.Copy(vd_b.get(), m_vd.get());

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

std::shared_ptr<VolumeData> VolumeSelector::GetResult(bool pop)
{
	std::shared_ptr<VolumeData> vd;
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
	SelectMode mode = m_mode;
	if (mode == SelectMode::SingleSelect ||
		mode == SelectMode::Segment ||
		mode == SelectMode::Mesh)
		mode = SelectMode::Append;
	for (int i = 0; i < m_brush_radius_sets.size(); ++i)
	{
		BrushRadiusSet& radius_set = m_brush_radius_sets.at(i);
		if (radius_set.type == mode)
		{
			//get new
			m_brush_radius1 = radius_set.radius1;
			m_brush_radius2 = radius_set.radius2;
			m_use_brush_radius2 = radius_set.use_radius2;
			m_iter_num = radius_set.iter_num;
			m_brush_sets_index = i;
			break;
		}
	}
}

//th udpate
bool VolumeSelector::GetThUpdate()
{
	auto view = glbin_current.render_view.lock();
	if (!view ||
		(m_mode != SelectMode::SingleSelect &&
		m_mode != SelectMode::Append &&
		m_mode != SelectMode::Diffuse))
		return false;
	glm::mat4 mv_mat = view->GetDrawMat();
	glm::mat4 prj_mat = view->GetProjection();
	//compare view
	if (mv_mat == m_mv_mat && prj_mat == m_prj_mat)
		return true;
	else
		return false;
}

bool VolumeSelector::GetAutoThreshold()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd)
		return false;
	if (m_vd->IsAutoThresholdValid())
		return false;
	double threshold = m_vd->GetAutoThreshold();
	if (threshold >= 0.0 && threshold != m_scl_translate)
	{
		m_scl_translate = threshold;
		glbin_comp_generator.SetThresh(m_scl_translate);
		glbin_conv_vol_mesh->SetIsoValue(m_scl_translate);
		return true;
	}
	return false;
}

void VolumeSelector::PushMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd || !m_vd->GetTexture())
		return;
	if (flvr::Texture::mask_undo_num_ > 0)
		m_vd->GetTexture()->push_mask();
}

void VolumeSelector::PopMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->pop_mask();
	m_vd->GetVR()->clear_tex_mask();
	m_vd->ResetMaskCount();
}

void VolumeSelector::UndoMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_backward();
	m_vd->GetVR()->clear_tex_mask();
	m_vd->ResetMaskCount();
}

void VolumeSelector::RedoMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_forward();
	m_vd->GetVR()->clear_tex_mask();
	m_vd->ResetMaskCount();
}

//mask operations
void VolumeSelector::CopyMask(bool copy_data)
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (m_vd)
	{
		m_vd_copy = m_vd;
		m_copy_data = copy_data;
	}
}

void VolumeSelector::PasteMask(int op)
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
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
			auto group = glbin_current.vol_group.lock();
			Nrrd* data = m_vd->GetMask(false);
			if (group && data)
				group->AddMask(data, 0);
		}
	}
	m_vd->ResetMaskCount();
}

bool VolumeSelector::GetMouseVec(int mx, int my, fluo::Vector &mvec)
{
	//DBGPRINT(L"mx: %d\tmy: %d\n", mx, my);
	auto view = glbin_current.render_view.lock();
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!view || !m_vd)
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
	
	int nx = view->GetCanvasSize().w();
	int ny = view->GetCanvasSize().h();
	fluo::Transform *tform = m_vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	glm::mat4 mv_mat = view->GetDrawMat();
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