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

#include "DrawVolumeDepth.hpp"
#include <VolumeGroup.hpp>
#include <VolumeData.hpp>

using namespace fluo;

DrawVolumeDepth::DrawVolumeDepth():
	Renderer3D()
{
}

DrawVolumeDepth::DrawVolumeDepth(const DrawVolumeDepth& renderer, const CopyOp& copyop, bool copy_values):
	Renderer3D(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
}

DrawVolumeDepth::~DrawVolumeDepth()
{
}

void DrawVolumeDepth::render(Event& event)
{
	VolumeGroup* group;
	getValue("volume group", (Referenced**)&group);
	if (!group || !group->getNumChildren())
		return;

	glm::mat4 mv_mat, proj_mat, tex_mat;
	bool use_fog;
	double fog_intensity, fog_start, fog_end;
	getValue("use fog", use_fog);
	getValue("fog intensity", fog_intensity);
	getValue("fog start", fog_start);
	getValue("fog end", fog_end);
	double zoom;
	getValue("scale factor", zoom);
	double sf121;// = Get121ScaleFactor();
	getValue("scale factor 121", sf121);
	VolumeData* data0 = 0;

	renderer_.clear_vr();
	for (size_t i = 0; i < group->getNumChildren(); ++i)
	{
		VolumeData* data = dynamic_cast<VolumeData*>(group->getChild(i));
		if (data)
		{
			bool disp;
			data->getValue("disp", disp);
			if (disp)
			{
				flvr::VolumeRenderer* vr = data->GetRenderer();
				if (vr)
				{
					//VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
					//if (vr_frame &&
					//	vr_frame->GetSettingDlg() &&
					//	vr_frame->GetSettingDlg()->GetRunScript() &&
					//	vd->GetMask(false) &&
					//	vd->GetLabel(false))
					//	vd->SetMaskMode(4);
					long mask_mode;
					data->getValue("mask mode", mask_mode);
					vr->set_ml_mode(mask_mode);
					vr->set_matrices(mv_mat, proj_mat, tex_mat);
					vr->set_fog(use_fog, fog_intensity, fog_start, fog_end);
					vr->set_zoom(zoom, sf121);
					renderer_.add_vr(vr);
				}
				if (!data0)
					data0 = data;
			}
		}
	}
	if (renderer_.get_vr_num() <= 0 || !data0)
		return;

	// Set up transform
	fluo::Transform *tform = data0->GetTexture()->transform();
	float mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_mat2 = data0->GetRenderer()->m_mv_mat * mv_mat2;
	renderer_.set_matrices(mv_mat2,
		data0->GetRenderer()->m_proj_mat,
		data0->GetRenderer()->m_tex_mat);
	//set up other settings
	long nx, ny;
	getValue("nx", nx);
	getValue("ny", ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };
	bool blend_slices;
	getValue("blend slices", blend_slices);
	bool peel;
	getValue("peel", peel);
	double sample_rate;
	getValue("sample rate", sample_rate);
	bool noise_red;
	getValue("noise red", noise_red);
	long cur_framebuffer;
	getValue("cur framebuffer", cur_framebuffer);
	bool test_wiref, adaptive, interactive, persp, intp;
	getValue("test wiref", test_wiref);
	getValue("adaptive", adaptive);
	getValue("interactive", interactive);
	getValue("persp", persp);
	getValue("intp", intp);

	renderer_.set_blend_slices(blend_slices);
	renderer_.set_sampling_rate(sample_rate);
	renderer_.SetNoiseRed(noise_red);
	renderer_.set_depth_peel(peel);
	renderer_.set_viewport(vp);
	renderer_.set_clear_color(clear_color);
	renderer_.set_cur_framebuffer(cur_framebuffer);

	//draw multiple volumes at the same time
	renderer_.draw(test_wiref, adaptive, interactive, !persp, intp);
	//generate textures & buffer objects
	//frame buffer for each volume
	//flvr::Framebuffer* chann_buffer =
	//	flvr::TextureRenderer::framebuffer_manager_.framebuffer(
	//		flvr::FB_Render_RGBA, nx, ny, "channel");
	////bind the fbo
	//if (chann_buffer)
	//{
	//	chann_buffer->protect();
	//	chann_buffer->bind();
	//	m_cur_framebuffer = chann_buffer->id();
	//}
	//if (!flvr::TextureRenderer::get_mem_swap() ||
	//	(flvr::TextureRenderer::get_mem_swap() &&
	//		flvr::TextureRenderer::get_clear_chan_buffer()))
	//{
	//	glClearColor(0.0, 0.0, 0.0, 0.0);
	//	glClear(GL_COLOR_BUFFER_BIT);
	//	flvr::
	//		TextureRenderer::reset_clear_chan_buffer();
	//}


}