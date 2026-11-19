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

#include <ViewRenderer.h>
#include <RenderView.h>
#include <Global.h>
#include <LookingGlassRenderer.h>
#include <MainSettings.h>
#include <RenderSystem.h>
#include <Coordinator.h>
#include <SettingDlgAgent.h>

using namespace flrd;

void ViewRenderer::render()
{
	auto view = context_->view.lock();
	if (!view)
		return;
	view_ = view.get();

	m_force_clear = context_->clearFramebuffer;
	m_interactive = context_->interactive;
	m_resizing = context_->resizing;
	if (context_->restartLoop)
		StartLoopUpdate();
	SetSortBricks();
	glbin_lg_renderer.SetUpdating(context_->lgChanged);
	Draw();
	DrawDefault();
}

bool ViewRenderer::Draw()
{
	m_updating = true;

	bool swap = false;

	flvr::RenderSystem::InitViewRenderer();
	view_->Init();

	switch (glbin_settings.m_hologram_mode)
	{
	case 0:
		break;
	case 1:
		flvr::RenderSystem::InitOpenXR();
		if (flvr::RenderSystem::openxr_initialized_)
		{
			view_->SetPersp(true);// always use perspective for xr
		}
		else
		{
			glbin_settings.m_xr_api = 0;
			auto setting_agent = glbin_coordinator.FindAgent<SettingDlgAgent>();
			if (setting_agent)
				setting_agent->UpdateDataToUI({ gstHologramMode });
		}
		break;
	case 2:
		flvr::RenderSystem::InitLookingGlass();
		break;
	}

	if (m_resizing)
		view_->m_retain_finalbuffer = false;

	int nx, ny;
	view_->GetRenderSize(nx, ny);

	PopMeshList();
	if (glbin_states.QuerySelection())
		m_draw_type = 1;
	else if (m_md_pop_list.size() > 0)
		m_draw_type = 2;
	else
		m_draw_type = 1;

	m_drawing = true;
	PreDraw();

	if (glbin_settings.m_hologram_mode == 1)
		PrepareStereoFramebuffer();
	BindViewBaseFramebuffer();
	ClearViewBaseFramebuffer();

	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);

	//gradient background
	if (glbin_settings.m_grad_bg)
		DrawGradBg();

	if (!m_draw_all)
		return true;

	m_mv_mat = GetDrawMat();

	if (glbin_settings.m_test_wiref)
		DrawGrid();

	if (glbin_states.m_clip_display)
		DrawClippingPlanes(flvr::FaceWinding::Back);

	switch (m_draw_type)
	{
	case 1:  //draw volumes only
		DrawData();
		break;
	case 2:  //draw volumes and mesh with depth peeling
		DrawDataPeel();
		break;
	}

	if (glbin_states.m_clip_display)
		DrawClippingPlanes(flvr::FaceWinding::Front);

	if (m_draw_camctr)
		DrawCamCtr();

	if (m_draw_legend)
		DrawLegend();

	DrawScaleBar();

	DrawColormap();

	PostDraw();

	//draw frame after capture
	if (m_draw_frame)
		DrawFrame();

	//draw info
	if (m_draw_info & INFO_DISP)
		DrawInfo(nx, ny, m_interactive);

	if (m_int_mode == InteractiveMode::BrushSelect ||
		m_int_mode == InteractiveMode::BrushRuler)  //painting mode
	{
		if (m_draw_brush)
		{
			DrawBrushOutlines();
		}
		if (m_paint_enable)
		{
			//paiting mode
			//for volume segmentation
			GenerateBrushStrokes();
		}
		if (m_paint_enable && m_paint_display)
		{
			//show the paint strokes
			DrawBrushStrokes();
		}
	}

	if (m_int_mode == InteractiveMode::BrushSelectUpdate)
		m_int_mode = InteractiveMode::BrushSelect;
	if (m_int_mode == InteractiveMode::BrushRulerUpdate)
		m_int_mode = InteractiveMode::BrushRuler;

	//swap
	if (glbin_settings.m_hologram_mode == 1)
	{
		if (m_vr_eye_idx)
		{
			DrawHologramFramebuffer();
			m_vr_eye_idx = 0;
			swap = true;
		}
		else
		{
			m_vr_eye_idx = 1;
			glbin_refresh_scheduler_manager.requestDraw(DrawRequest("VR right refresh", { static_cast<int>(m_id) }));
		}
	}
	else if (glbin_settings.m_hologram_mode == 2)
	{
		glbin_lg_renderer.Draw();
		swap = true;
	}
	else
		swap = true;

	glbin.getStopWatch(gstStopWatch)->sample();
	m_drawing = false;

	if (m_resize)
		m_resize = false;

	if (m_enlarge)
		ResetEnlarge();

	if (m_interactive)
	{
		//the draw request is non interactive, which resets the flag
		glbin_refresh_scheduler_manager.requestDraw(DrawRequest("Interctive update refresh", { static_cast<int>(m_id) }));
	}

	return swap;
}

void ViewRenderer::DrawDefault()
{
	auto default_buffer = glbin_framebuffer_manager.framebuffer(
		flvr::FBRole::Canvas,
		static_cast<int>(std::round(m_canvas_size.w() * m_dpi_factor)),
		static_cast<int>(std::round(m_canvas_size.h() * m_dpi_factor)),
		gstRBCanvasDefault);
	assert(default_buffer);
	glbin_framebuffer_manager.bind(default_buffer);

	auto img_shader = glbin_shader_manager.shader(gstImgShader,
		flvr::ShaderParams::Img(IMG_SHDR_TEXTURE_LOOKUP, 0));
	assert(img_shader);
	img_shader->bind();
	auto base_buffer = glbin_framebuffer_manager.framebuffer(gstRBViewBase);
	assert(base_buffer);
	base_buffer->bind_texture(flvr::AttachmentPoint::Color(0), 0);
	DrawViewQuad();
	img_shader->unbind();
	base_buffer->unbind_texture(flvr::AttachmentPoint::Color(0));
}

//start loop update
void ViewRenderer::StartLoopUpdate()
{
	////this is for debug_ds, comment when done
	//if (TextureRenderer::get_mem_swap() &&
	//  TextureRenderer::get_start_update_loop() &&
	//  !TextureRenderer::get_done_update_loop())
	//  return;

	if (glbin_settings.m_mem_swap)
	{
		if (flvr::TextureRenderer::active_view_ > 0 &&
			flvr::TextureRenderer::active_view_ != Id())
			return;
		else
			flvr::TextureRenderer::active_view_ = Id();

		int nx, ny;
		GetRenderSize(nx, ny);
		//projection
		HandleProjection(nx, ny, true);
		//Transformation
		HandleCamera(true);
		glm::mat4 mv_temp = m_mv_mat;
		m_mv_mat = GetDrawMat();

		PopVolumeList();
		int total_num = 0;
		int num_chan;
		//reset drawn status for all bricks
		for (auto it = m_vd_pop_list.begin(); it != m_vd_pop_list.end(); ++it)
		{
			auto vd = it->lock();
			if (!vd)
				continue;

			switchLevel(vd.get());
			vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);

			num_chan = 0;
			flvr::Texture* tex = vd->GetTexture();
			if (tex)
			{
				fluo::Ray view_ray = vd->GetVR()->compute_view();
				std::vector<flvr::TextureBrick*>* bricks = 0;
				bricks = tex->get_sorted_bricks(view_ray, !m_persp);
				if (!bricks || bricks->size() == 0)
					continue;
				for (size_t j = 0; j < bricks->size(); j++)
				{
					(*bricks)[j]->set_drawn(false);
					if ((*bricks)[j]->get_priority() > 0 ||
						!vd->GetVR()->test_against_view((*bricks)[j]->bbox(), m_persp))
					{
						(*bricks)[j]->set_disp(false);
						continue;
					}
					else
						(*bricks)[j]->set_disp(true);
					total_num++;
					num_chan++;
					if (vd->GetRenderMode() == flvr::RenderMode::Mip &&
						vd->GetShadingEnable())
						total_num++;
				}
			}
			vd->SetBrickNum(num_chan);
			if (vd->GetVR())
				vd->GetVR()->set_done_loop(false);
		}

		std::vector<VolumeLoaderData> queues;
		if (m_channel_mix_mode == ChannelMixMode::Depth)
		{
			std::vector<std::shared_ptr<VolumeData>> list;
			for (auto it = m_vd_pop_list.begin(); it != m_vd_pop_list.end(); ++it)
			{
				auto vd = it->lock();
				if (!vd || !vd->GetDisp() || !vd->isBrxml())
					continue;
				flvr::Texture* tex = vd->GetTexture();
				if (!tex)
					continue;
				std::vector<flvr::TextureBrick*>* bricks = tex->get_bricks();
				if (!bricks || bricks->size() == 0)
					continue;
				list.push_back(vd);
			}

			std::vector<VolumeLoaderData> tmp_shade;
			std::vector<VolumeLoaderData> tmp_shadow;
			for (auto& vd : list)
			{
				flvr::Texture* tex = vd->GetTexture();
				fluo::Ray view_ray = vd->GetVR()->compute_view();
				std::vector<flvr::TextureBrick*>* bricks = tex->get_sorted_bricks(view_ray, !m_persp);
				int mode = vd->GetRenderMode() == flvr::RenderMode::Mip ? 1 : 0;
				bool shade = (mode == 1 && vd->GetShadingEnable());
				bool shadow = vd->GetShadowEnable();
				for (size_t j = 0; j < bricks->size(); ++j)
				{
					VolumeLoaderData d = {};
					flvr::TextureBrick* b = (*bricks)[j];
					if (b->get_disp())
					{
						d.brick = b;
						d.finfo = tex->GetFileName(b->getID());
						d.vd = vd.get();
						if (!b->drawn(mode))
						{
							d.mode = mode;
							queues.push_back(d);
						}
						if (shade && !b->drawn(2))
						{
							d.mode = 2;
							tmp_shade.push_back(d);
						}
						if (shadow && !b->drawn(3))
						{
							d.mode = 3;
							tmp_shadow.push_back(d);
						}
					}
				}
			}
			if (glbin_settings.m_update_order == 1)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_dsc);
			else if (glbin_settings.m_update_order == 0)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_asc);

			if (!tmp_shade.empty())
			{
				if (glbin_settings.m_update_order == 1)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
				else if (glbin_settings.m_update_order == 0)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
				queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
			}
			if (!tmp_shadow.empty())
			{
				if (glbin_settings.m_update_order == 1)
				{
					int order = glbin_settings.m_update_order;
					glbin_settings.m_update_order = 0;
					for (size_t i = 0; i < list.size(); i++)
					{
						fluo::Ray view_ray = list[i]->GetVR()->compute_view();
						list[i]->GetTexture()->set_sort_bricks();
						list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
						list[i]->GetTexture()->set_sort_bricks();
					}
					glbin_settings.m_update_order = order;
					std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
				}
				else if (glbin_settings.m_update_order == 0)
					std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
				queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
			}
		}
		else if (!m_layer_list.empty())
		{
			for (auto it = m_layer_list.rbegin(); it != m_layer_list.rend(); ++it)
			{
				if (!(*it))
					continue;
				switch ((*it)->IsA())
				{
				case 2://volume data (this won't happen now)
				{
					auto vd = std::dynamic_pointer_cast<VolumeData>(*it);
					std::vector<VolumeLoaderData> tmp_shade;
					std::vector<VolumeLoaderData> tmp_shadow;
					if (vd && vd->GetDisp() && vd->isBrxml())
					{
						flvr::Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						fluo::Ray view_ray = vd->GetVR()->compute_view();
						std::vector<flvr::TextureBrick*>* bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						int mode = vd->GetRenderMode() == flvr::RenderMode::Mip ? 1 : 0;
						bool shade = (mode == 1 && vd->GetShadingEnable());
						bool shadow = vd->GetShadowEnable();
						for (size_t j = 0; j < bricks->size(); j++)
						{
							VolumeLoaderData d = {};
							flvr::TextureBrick* b = (*bricks)[j];
							if (b->get_disp())
							{
								d.brick = b;
								d.finfo = tex->GetFileName(b->getID());
								d.vd = vd.get();
								if (!b->drawn(mode))
								{
									d.mode = mode;
									queues.push_back(d);
								}
								if (shade && !b->drawn(2))
								{
									d.mode = 2;
									tmp_shade.push_back(d);
								}
								if (shadow && !b->drawn(3))
								{
									d.mode = 3;
									tmp_shadow.push_back(d);
								}
							}
						}
						if (!tmp_shade.empty()) queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
						if (!tmp_shadow.empty())
						{
							if (glbin_settings.m_update_order == 1)
							{
								int order = glbin_settings.m_update_order;
								glbin_settings.m_update_order = 0;
								fluo::Ray view_ray = vd->GetVR()->compute_view();
								tex->set_sort_bricks();
								tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
								tex->set_sort_bricks();
								glbin_settings.m_update_order = order;
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							}
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
				}
				break;
				case 5://group
				{
					std::vector<std::shared_ptr<VolumeData>> list;
					auto group = std::dynamic_pointer_cast<VolumeGroup>(*it);
					if (!group->GetDisp())
						continue;
					for (int j = group->GetVolumeNum(); j > 0; --j)
					{
						auto vd = group->GetVolumeData(j - 1);
						if (!vd || !vd->GetDisp() || !vd->isBrxml())
							continue;
						flvr::Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						fluo::Ray view_ray = vd->GetVR()->compute_view();
						std::vector<flvr::TextureBrick*>* bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						list.push_back(vd);
					}
					if (list.empty())
						continue;

					std::vector<VolumeLoaderData> tmp_q;
					std::vector<VolumeLoaderData> tmp_shade;
					std::vector<VolumeLoaderData> tmp_shadow;
					if (group->GetChannelMixMode() == ChannelMixMode::Depth)
					{
						for (size_t k = 0; k < list.size(); k++)
						{
							auto vd = list[k];
							flvr::Texture* tex = vd->GetTexture();
							fluo::Ray view_ray = vd->GetVR()->compute_view();
							std::vector<flvr::TextureBrick*>* bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetRenderMode() == flvr::RenderMode::Mip ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShadingEnable());
							bool shadow = vd->GetShadowEnable();
							for (size_t j = 0; j < bricks->size(); j++)
							{
								VolumeLoaderData d = {};
								flvr::TextureBrick* b = (*bricks)[j];
								if (b->get_disp())
								{
									d.brick = b;
									d.finfo = tex->GetFileName(b->getID());
									d.vd = vd.get();
									if (!b->drawn(mode))
									{
										d.mode = mode;
										tmp_q.push_back(d);
									}
									if (shade && !b->drawn(2))
									{
										d.mode = 2;
										tmp_shade.push_back(d);
									}
									if (shadow && !b->drawn(3))
									{
										d.mode = 3;
										tmp_shadow.push_back(d);
									}
								}
							}
						}
						if (!tmp_q.empty())
						{
							if (glbin_settings.m_update_order == 1)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_dsc);
							else if (glbin_settings.m_update_order == 0)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_q.begin(), tmp_q.end());
						}
						if (!tmp_shade.empty())
						{
							if (glbin_settings.m_update_order == 1)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
							else if (glbin_settings.m_update_order == 0)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
						}
						if (!tmp_shadow.empty())
						{
							if (glbin_settings.m_update_order == 1)
							{
								int order = glbin_settings.m_update_order;
								glbin_settings.m_update_order = 0;
								for (size_t k = 0; k < list.size(); k++)
								{
									fluo::Ray view_ray = list[k]->GetVR()->compute_view();
									list[k]->GetTexture()->set_sort_bricks();
									list[k]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									list[k]->GetTexture()->set_sort_bricks();
								}
								glbin_settings.m_update_order = order;
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							}
							else if (glbin_settings.m_update_order == 0)
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
					else
					{
						for (size_t j = 0; j < list.size(); j++)
						{
							auto vd = list[j];
							flvr::Texture* tex = vd->GetTexture();
							fluo::Ray view_ray = vd->GetVR()->compute_view();
							std::vector<flvr::TextureBrick*>* bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetRenderMode() == flvr::RenderMode::Mip ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShadingEnable());
							bool shadow = vd->GetShadowEnable();
							for (size_t k = 0; k < bricks->size(); k++)
							{
								VolumeLoaderData d = {};
								flvr::TextureBrick* b = (*bricks)[k];
								if (b->get_disp())
								{
									d.brick = b;
									d.finfo = tex->GetFileName(b->getID());
									d.vd = vd.get();
									if (!b->drawn(mode))
									{
										d.mode = mode;
										queues.push_back(d);
									}
									if (shade && !b->drawn(2))
									{
										d.mode = 2;
										tmp_shade.push_back(d);
									}
									if (shadow && !b->drawn(3))
									{
										d.mode = 3;
										tmp_shadow.push_back(d);
									}
								}
							}
							if (!tmp_shade.empty()) queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
							if (!tmp_shadow.empty())
							{
								if (glbin_settings.m_update_order == 1)
								{
									int order = glbin_settings.m_update_order;
									glbin_settings.m_update_order = 0;
									fluo::Ray view_ray = vd->GetVR()->compute_view();
									tex->set_sort_bricks();
									tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									tex->set_sort_bricks();
									glbin_settings.m_update_order = order;
									std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
								}
								queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
							}
						}
					}

				}
				break;
				}
			}
		}

		if (queues.size() > 0 /*&& !m_interactive*/)
		{
			glbin_vol_loader.Set(queues);
			glbin_vol_loader.SetMemoryLimitByte((long long)flvr::TextureRenderer::mainmem_buf_size_ * 1024LL * 1024LL);
			flvr::TextureRenderer::set_load_on_main_thread(false);
			glbin_vol_loader.Run();
		}

		if (total_num > 0)
		{
			flvr::TextureRenderer::set_update_loop();
			if (m_draw_type == 1)
				flvr::TextureRenderer::set_total_brick_num(total_num);
			else if (m_draw_type == 2)
				flvr::TextureRenderer::set_total_brick_num(total_num * (glbin_settings.m_peeling_layers + 1));
			flvr::TextureRenderer::reset_done_current_chan();
		}
	}
}

//halt loop update
void ViewRenderer::HaltLoopUpdate()
{
	if (glbin_settings.m_mem_swap)
	{
		flvr::TextureRenderer::reset_update_loop();
	}
}

//sort bricks after view changes
void ViewRenderer::SetSortBricks()
{
	PopVolumeList();

	for (auto it = m_vd_pop_list.begin(); it != m_vd_pop_list.end(); ++it)
	{
		auto vd = it->lock();
		if (vd && vd->GetTexture())
			vd->GetTexture()->set_sort_bricks();
	}
}

