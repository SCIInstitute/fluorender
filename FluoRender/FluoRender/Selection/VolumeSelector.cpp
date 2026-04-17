/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <BrushDefault.h>
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

bool VolumeSelector::GetAutoPaintSize() const
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
		flrd::Histogram histogram(m_vd);
		histogram.SetProgressFunc(glbin_data_manager.GetProgressFunc());
		histogram.SetUseMask(true);
		auto eh = histogram.GetEntryHist();

		if (eh)
		{
			//record
			auto rec = std::make_shared<flrd::RecordHistParams>();
			rec->setInput(eh);
			auto ep = std::make_shared<flrd::EntryParams>(glbin.get_cg_entry());
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

	if (m_mode == SelectMode::Segment)
	{
		//generate components
		if (m_vd->IsValidMask())
		{
			bool bval = glbin_comp_generator.GetUseSel();
			glbin_comp_generator.SetUseSel(true);
			glbin_comp_generator.GenerateComp();
			glbin_comp_generator.SetUseSel(bval);
		}
	}

	if (m_mode == SelectMode::Mesh)
	{
		//generate mesh
		glbin_conv_vol_mesh.SetVolumeData(m_vd);
		glbin_conv_vol_mesh.SetUseMask(true);
		glbin_conv_vol_mesh.Update(true);
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
		m_vd->GetVR().set_mouse_position(mp);
		valid_mvec = GetMouseVec(mx, my, mvec);
	}
	m_vd->GetVR().set_mouse_vec(mvec);

	auto paint_buffer = glbin_framebuffer_manager.framebuffer(gstRBPaintBrush);
	if (paint_buffer)//paint buffer can be empty for grow tool
		Set2DMask(paint_buffer->tex_id(flvr::AttachmentPoint::Color(0)));
	auto final_buffer = glbin_framebuffer_manager.framebuffer(gstRBViewData);
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
	auto bricks = m_vd->GetTexture()->get_bricks();
	if (m_mode == SelectMode::SingleSelect ||
		m_mode == SelectMode::Append ||
		m_mode == SelectMode::Eraser ||
		m_mode == SelectMode::Diffuse ||
		m_mode == SelectMode::Solid ||
		m_mode == SelectMode::Grow ||
		m_mode == SelectMode::Segment ||
		m_mode == SelectMode::Mesh)
	{
		if (bricks.size() > 1)
		{
			flrd::PaintBoxes pb;
			pb.SetBricks(bricks);
			pb.SetPersp(!view->GetPersp());
			fluo::Transform* tform = m_vd->GetTexture()->transform();
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
			flrd::MaskBorder mb(m_vd);
			for (int i = 0; i < m_iter; i++)
			{
				order = m_update_order ? (i % div) : 0;
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

	if (flvr::Texture::mask_undo_num_ > 0)
		m_vd->GetVR().return_mask();

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
		(select && !m_vd->GetTexture()->has_comp(flvr::CompType::Mask)) ||
		!m_vd->GetTexture()->has_comp(flvr::CompType::Label))
		return;

	if (select)
		m_vd->GetVR().return_mask();

	//get all the data from original volume
	auto raw_data = m_vd->GetVolume(false);
	if (!raw_data) return;
	auto raw_mask = m_vd->GetMask(false);
	if (select && !raw_mask) return;
	auto raw_label = m_vd->GetLabel(false);
	if (!raw_label) return;
	void* data_mvd = raw_data->GetDataVoid();
	unsigned char* data_mvd_mask = select ? raw_mask->GetData() : 0;
	unsigned int* data_mvd_label = raw_label->DataAs<unsigned int>();
	if (!data_mvd || (select && !data_mvd_mask) || !data_mvd_label) return;

	//create new volumes
	int bits = 8;
	auto res = m_vd->GetResolution();
	auto spc = m_vd->GetSpacing();
	int brick_size = m_vd->GetTexture()->get_build_max_tex_size();
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();

	bool push_new = true;
	//red volume
	if (!vd_r)
		vd_r = std::make_shared<VolumeData>();
	else
		push_new = false;
	vd_r->AddEmptyData(bits,
		res,
		spc,
		brick_size);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(m_vd->GetName() + L"_COMP1");
	vd_r->SetCurChannel(0);
	//green volume
	if (!vd_g)
		vd_g = std::make_shared<VolumeData>();
	vd_g->AddEmptyData(bits,
		res,
		spc,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_vd->GetName() + L"_COMP2");
	vd_g->SetCurChannel(1);
	//blue volume
	if (!vd_b)
		vd_b = std::make_shared<VolumeData>();
	vd_b->AddEmptyData(bits,
		res,
		spc,
		brick_size);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(m_vd->GetName() + L"_COMP3");
	vd_b->SetCurChannel(2);

	//get new data
	//red volume
	auto raw_vd_r = vd_r->GetVolume(false);
	if (!raw_vd_r) return;
	//green volume
	auto raw_vd_g = vd_g->GetVolume(false);
	if (!raw_vd_g) return;
	//blue volume
	auto raw_vd_b = vd_b->GetVolume(false);
	if (!raw_vd_b) return;

	if (hide)
		m_randv = int((double)std::rand() / (RAND_MAX) * 900 + 100);

	const double scalar_scale = m_vd->GetScalarScale();

	auto colorize = [this, select, scalar_scale, hmode](uint32_t label_value,
		auto intensity_value,
		uint8_t mask_value,
		uint8_t& out_r,
		uint8_t& out_g,
		uint8_t& out_b)
		{
			if (label_value == 0)
				return;

			// ----- normalize intensity (exact old behavior) -----
			double value = 0.0;

			using IT = std::decay_t<decltype(intensity_value)>;

			if constexpr (std::is_same_v<IT, uint8_t>)
			{
				value = select
					? (double(intensity_value) * double(mask_value)) / 65025.0
					: double(intensity_value) / 255.0;
			}
			else if constexpr (std::is_same_v<IT, uint16_t>)
			{
				value = select
					? (double(intensity_value) * scalar_scale *
						double(mask_value)) / 16581375.0
					: (double(intensity_value) * scalar_scale) / 65535.0;
			}

			value = std::min(value, 1.0);

			// ----- label → color -----
			fluo::Color color;
			if (hmode == 0)
			{
				color = fluo::Color(label_value, m_vd->GetShuffle());
			}
			else
			{
				double hue = HueCalculation(hmode, label_value);
				color = fluo::Color(fluo::HSVColor(hue, 1.0, 1.0));
			}

			// ----- write outputs -----
			out_r = static_cast<uint8_t>(color.r() * 255.0 * value);
			out_g = static_cast<uint8_t>(color.g() * 255.0 * value);
			out_b = static_cast<uint8_t>(color.b() * 255.0 * value);
		};
	//populate the data
	const size_t N = raw_label->GetElementCount();

	auto* lbl = raw_label->DataAs<uint32_t>();
	auto* dat = raw_data->GetDataVoid();     // type-dispatched below
	auto* msk = raw_mask ? raw_mask->DataAs<uint8_t>() : nullptr;

	auto* r = raw_vd_r->DataAs<uint8_t>();
	auto* g = raw_vd_g->DataAs<uint8_t>();
	auto* b = raw_vd_b->DataAs<uint8_t>();

	switch (raw_data->GetFormat())
	{
	case fluo::DataFormat::UInt8:
	{
		auto* d = raw_data->DataAs<uint8_t>();
		for (size_t i = 0; i < N; ++i)
			colorize(lbl[i], d[i], msk ? msk[i] : 255, r[i], g[i], b[i]);
		break;
	}
	case fluo::DataFormat::UInt16:
	{
		auto* d = raw_data->DataAs<uint16_t>();
		for (size_t i = 0; i < N; ++i)
			colorize(lbl[i], d[i], msk ? msk[i] : 255, r[i], g[i], b[i]);
		break;
	}
	default:
		break;
	}

	glbin_vol_def.Copy(*vd_r, *m_vd);
	glbin_vol_def.Copy(*vd_g, *m_vd);
	glbin_vol_def.Copy(*vd_b, *m_vd);

	fluo::Color red(1.0, 0.0, 0.0);
	fluo::Color green(0.0, 1.0, 0.0);
	fluo::Color blue(0.0, 0.0, 1.0);
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

std::shared_ptr<VolumeData> VolumeSelector::GetResult(bool pop) const
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
void VolumeSelector::ChangeBrushSize(int value, bool ctrl)
{
	if (!value) return;

	if (m_mode == SelectMode::Solid || !m_use_brush_radius2)
	{
		double delta = value * m_brush_radius1 / 1000.0;
		m_brush_radius1 += delta;
		m_brush_radius1 = std::max(m_brush_radius1, 1.0);
		m_brush_radius2 = m_brush_radius1;
	}
	else
	{
		if (ctrl)
		{
			double delta = value * m_brush_radius1 / 1000.0;
			m_brush_radius1 += delta;
			m_brush_radius1 = std::max(m_brush_radius1, 1.0);
			m_brush_radius2 = std::max(m_brush_radius2, m_brush_radius1);
		}
		else
		{
			double delta = value * m_brush_radius2 / 2000.0;
			m_brush_radius2 += delta;
			m_brush_radius2 = std::max(1.0, m_brush_radius2);
			m_brush_radius1 = std::min(m_brush_radius2, m_brush_radius1);
		}
	}

	UpdateBrushRadiusSet();
}

//brush sets
void VolumeSelector::GetBrushRadiusSet(std::vector<BrushRadiusSet>& sets) const
{
	sets.assign(m_brush_radius_sets.begin(), m_brush_radius_sets.end());
}

void VolumeSelector::SetBrushRadiusSet(const std::vector<BrushRadiusSet>& sets)
{
	m_brush_radius_sets.assign(sets.begin(), sets.end());
	if (!m_brush_radius_sets.empty() &&
		m_brush_sets_index >= 0 &&
		m_brush_sets_index < m_brush_radius_sets.size())
	{
		BrushRadiusSet& radius_set = m_brush_radius_sets.at(m_brush_sets_index);
		m_brush_radius1 = radius_set.radius1;
		m_brush_radius2 = radius_set.radius2;
		m_use_brush_radius2 = radius_set.use_radius2;
		m_iter_num = radius_set.iter_num;
	}
}

void VolumeSelector::UpdateBrushRadiusSet()
{
	SelectMode mode = m_mode;
	if (mode == SelectMode::SingleSelect ||
		mode == SelectMode::Segment ||
		mode == SelectMode::Mesh)
		mode = SelectMode::Append;
	for (auto& it : m_brush_radius_sets)
	{
		if (it.type == mode)
		{
			it.radius1 = m_brush_radius1;
			it.radius2 = m_brush_radius2;
			it.use_radius2 = m_use_brush_radius2;
			it.iter_num = m_iter_num;
		}
	}
}

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
bool VolumeSelector::GetThUpdate() const
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

bool VolumeSelector::GetAutoThreshold() const
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
		glbin_conv_vol_mesh.SetIsoValue(m_scl_translate);
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
	m_vd->GetVR().clear_tex_mask();
	m_vd->ResetMaskCount();
}

void VolumeSelector::UndoMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_backward();
	m_vd->GetVR().clear_tex_mask();
	m_vd->ResetMaskCount();
}

void VolumeSelector::RedoMask()
{
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!m_vd || !m_vd->GetTexture())
		return;

	m_vd->GetTexture()->mask_undos_forward();
	m_vd->GetVR().clear_tex_mask();
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
			auto raw_data = m_vd_copy->GetVolume(false);
			if (m_vd_copy->GetBits() == 16)
				m_vd->AddMaskConvert(raw_data, op, m_vd_copy->GetScalarScale());
			else
				m_vd->AddMask(raw_data, op);
		}
		else
			m_vd->AddMask(m_vd_copy->GetMask(false), op);

		if (m_select_multi)
		{
			auto group = glbin_current.vol_group.lock();
			auto raw_mask = m_vd->GetMask(false);
			if (group && raw_mask)
				group->AddMask(raw_mask, 0);
		}
	}
	m_vd->ResetMaskCount();
}

bool VolumeSelector::GetMouseVec(int mx, int my, fluo::Vector& mvec) const
{
	//DBGPRINT(L"mx: %d\tmy: %d\n", mx, my);
	auto view = glbin_current.render_view.lock();
	if (!m_vd)
		m_vd = glbin_current.vol_data.lock();
	if (!view || !m_vd)
		return false;
	if (mx >= 0 && my >= 0 &&
		m_mx0 >= 0 && m_my0 >= 0)
	{
		double dist = (m_mx - mx) * (m_mx - mx) + (m_my - my) * (m_my - my);
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
	fluo::Transform* tform = m_vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	glm::mat4 mv_mat = view->GetDrawMat();
	mv_mat = mv_mat * mv_mat2;
	glm::mat4 prj_mat = m_prj_mat;
	fluo::Transform mv, pr;
	mv.set(glm::value_ptr(mv_mat));
	pr.set(glm::value_ptr(prj_mat));
	mv.invert();
	pr.invert();
	fluo::Vector v;
	fluo::Point p0(double(m_mx0) * 2 / nx - 1.0, 1.0 - double(m_my0) * 2 / ny, 0);
	fluo::Point p1(double(m_mx) * 2 / nx - 1.0, 1.0 - double(m_my) * 2 / ny, 0);
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