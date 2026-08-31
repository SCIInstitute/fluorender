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

#include <MoviePanelAgent.h>
#include <MoviePanel.h>
#include <Global.h>
#include <Names.h>
#include <MovieMaker.h>
#include <Root.h>
#include <RenderView.h>
#include <DataManager.h>
#include <MovieDefault.h>
#include <MainSettings.h>
#include <Interpolator.h>

MoviePanelAgent::MoviePanelAgent(
	MoviePanel* panel) :
	Agent(panel)
{

}

bool MoviePanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MoviePanelAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void MoviePanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty() || FOUND_VALUE(gstMoviePanelAgent);
	bool bval;
	int ival;
	double dval;

	//modes
	if (update_all || FOUND_VALUE(gstMovFps))
	{
		dval = glbin_moviemaker.GetFps();
		panel->UpdateMovFps(dval);
	}

	if (update_all || FOUND_VALUE(gstMovLength))
	{
		dval = glbin_moviemaker.GetMovieLength();
		panel->UpdateMovLength(dval);
	}

	if (update_all || FOUND_VALUE(gstMovViewList))
	{
		MovViewListInfo info;
		Root* root = glbin_data_manager.GetRoot();
		if (root)
		{
			for (int i = 0; i < root->GetViewNum(); i++)
			{
				auto view = root->GetView(i);
				if (view)
					info.views.push_back(view->GetName());
			}
		}
	}
	if (update_all || FOUND_VALUE(gstMovViewIndex))
	{
		ival = glbin_mov_def.m_view_idx;
		panel->UpdateMovViewIndex(ival);
	}

	if (update_all || FOUND_VALUE(gstMovSliderStyle))
	{
		bval = glbin_mov_def.m_slider_style;
		panel->UpdateMovSliderStyle(bval);
	}

	if (update_all || FOUND_VALUE(gstMovProgSlider))
	{
		int cf = glbin_moviemaker.GetCurrentFrame();
		int ts = glbin_moviemaker.GetScrollThumbSize();
		int sf = glbin_moviemaker.GetClipStartFrame();
		int ef = glbin_moviemaker.GetClipEndFrame();
		panel->UpdateMovProgSlider(sf, ts, sf, ef);
	}

	if (update_all || FOUND_VALUE(gstBeginFrame))
	{
		ival = glbin_moviemaker.GetClipStartFrame();
		panel->UpdateBeginFrame(ival);
	}

	if (update_all || FOUND_VALUE(gstEndFrame))
	{
		ival = glbin_moviemaker.GetClipEndFrame();
		panel->UpdateEndFrame(ival);
	}

	if (update_all || FOUND_VALUE(gstCurrentFrame))
	{
		ival = glbin_moviemaker.GetCurrentFrame();
		panel->UpdateCurrentFrame(ival);
	}

	if (update_all || FOUND_VALUE(gstTotalFrames))
	{
		ival = glbin_moviemaker.GetFullFrameNum();
		panel->UpdateTotalFrames(ival);
	}

	if (update_all || FOUND_VALUE(gstMovCurTime))
	{
		dval = glbin_moviemaker.GetCurrentTime();
		panel->UpdateMovCurTime(dval);
	}

	if (update_all || FOUND_VALUE(gstMovPlay))
	{
		bool running = glbin_moviemaker.IsRunning();
		bool reverse = glbin_moviemaker.IsReverse();
		bool script = glbin_settings.m_run_script;
		panel->UpdateMovPlay(running, reverse, script);
	}

	if (update_all || FOUND_VALUE(gstMovLoop))
	{
		bval = glbin_moviemaker.IsLoop();
		panel->UpdateMovLoop(bval);
	}

	if (update_all || FOUND_VALUE(gstMovRotEnable))
	{
		bval = glbin_moviemaker.GetRotateEnable();
		panel->UpdateMovRotEnable(bval);
	}

	if (update_all || FOUND_VALUE(gstMovRotAxis))
	{
		ival = glbin_moviemaker.GetRotateAxis();
		panel->UpdateMovRotAxis(ival);
	}

	if (update_all || FOUND_VALUE(gstMovRotAng))
	{
		ival = glbin_moviemaker.GetRotateDeg();
		panel->UpdateMovRotAng(ival);
	}

	if (update_all || FOUND_VALUE(gstMovIntrpMode))
	{
		ival = glbin_moviemaker.GetInterpolation();
		panel->UpdateMovIntrpMode(ival);
	}

	if (update_all || FOUND_VALUE(gstMovSeqMode))
	{
		ival = glbin_moviemaker.GetSeqMode();
		panel->UpdateMovSeqMode(ival);
	}

	if (update_all || FOUND_VALUE(gstMovSeqNum))
	{
		int scn = glbin_moviemaker.GetSeqCurNum();
		int san = glbin_moviemaker.GetSeqAllNum();
		panel->UpdateMovSeqNum(scn, san);
	}

	if (update_all || FOUND_VALUE(gstCaptureParam))
	{
		bval = glbin_moviemaker.GetKeyframeEnable();
		panel->UpdateCaptureParam(bval);
	}

	if (update_all || FOUND_VALUE(gstParamKeyDuration))
	{
		dval = glbin_moviemaker.GetKeyDuration();
		panel->UpdateParamKeyDuration(dval);
	}

	if (update_all || FOUND_VALUE(gstParamList))
		panel->UpdateParamList();

	if (update_all || FOUND_VALUE(gstParamListSelect))
	{
		dval = glbin_moviemaker.GetCurProg();
		ival = glbin_interpolator.GetKeyIndexFromTime(dval);
		panel->UpdateParamListSelect(ival);
	}

	if (update_all || FOUND_VALUE(gstCamLockObjEnable))
		m_cam_lock_chk->SetValue(glbin_moviemaker.GetCamLock());

	if (update_all || FOUND_VALUE(gstCamLockType))
		m_cam_lock_cmb->SetSelection(glbin_moviemaker.GetCamLockType() - 1);

	if (update_all || FOUND_VALUE(gstCropEnable))
	{
		bval = glbin_moviemaker.GetCropEnable();
		m_crop_chk->SetValue(bval);
		m_crop_x_text->Enable(bval);
		m_crop_y_text->Enable(bval);
		m_crop_w_text->Enable(bval);
		m_crop_h_text->Enable(bval);

		m_sb_tl_rb->Enable(bval);
		m_sb_tr_rb->Enable(bval);
		m_sb_bl_rb->Enable(bval);
		m_sb_br_rb->Enable(bval);

		m_sb_dx_text->Enable(bval);
		m_sb_dx_spin->Enable(bval);
		m_sb_dy_text->Enable(bval);
		m_sb_dy_spin->Enable(bval);
	}

	if (update_all || FOUND_VALUE(gstCropValues))
	{
		m_crop_x_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropX()));
		m_crop_y_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropY()));
		m_crop_w_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropW()));
		m_crop_h_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropH()));
	}

	if (update_all || FOUND_VALUE(gstScalebarPos))
	{
		ival = glbin_moviemaker.GetScalebarPos();
		switch (ival)
		{
		case 0:
			m_sb_tl_rb->SetValue(true);
			break;
		case 1:
			m_sb_tr_rb->SetValue(true);
			break;
		case 2:
			m_sb_bl_rb->SetValue(true);
			break;
		case 3:
		default:
			m_sb_br_rb->SetValue(true);
			break;
		}
		m_sb_dx_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetScalebarX()));
		m_sb_dy_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetScalebarY()));
	}

	if (update_all || FOUND_VALUE(gstRunScript))
	{
		bval = glbin_settings.m_run_script;
		m_run_script_chk->SetValue(bval);
		size_t idx = 4;
		for (size_t i = 0; i < m_notebook->GetPageCount(); ++i)
		{
			wxString str = m_notebook->GetPageText(i);
			if (str.Contains(UITEXT_NBPG4_0))
			{
				idx = i;
				break;
			}
		}
		if (bval)
			m_notebook->SetPageText(idx, UITEXT_NBPG4_1);
		else
			m_notebook->SetPageText(idx, UITEXT_NBPG4_0);
	}

	if (update_all || FOUND_VALUE(gstScriptFile))
	{
		m_script_file_text->ChangeValue(glbin_settings.m_script_file);
	}

	if (update_all || FOUND_VALUE(gstScriptList))
	{
		m_script_list->DeleteAllItems();
		std::vector<std::wstring> list;
		std::wstring filename;
		long tmp;
		if (GetScriptFiles(list))
		{
			for (size_t i = 0; i < list.size(); ++i)
			{
				std::filesystem::path p(list[i]);
				filename = p.stem().wstring();
				tmp = m_script_list->InsertItem(i, std::to_wstring(i + 1), 0);
				m_script_list->SetItem(tmp, 1, filename);
			}
			m_script_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
			m_script_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
		}
	}

	if (update_all || FOUND_VALUE(gstScriptSelect))
	{
		std::vector<std::wstring> list;
		if (GetScriptFiles(list))
		{
			int idx = -1;
			for (size_t i = 0; i < list.size(); ++i)
			{
				if (glbin_settings.m_script_file == list[i])
				{
					idx = i;
					break;
				}
			}
			if (idx >= 0 && idx < m_script_list->GetItemCount())
			{
				m_script_list->SetItemState(idx,
					wxLIST_STATE_SELECTED,
					wxLIST_STATE_SELECTED);
				//wxSize ss = m_script_list->GetItemSpacing();
				//m_script_list->ScrollList(0, ss.y*idx);
			}
		}
	}
}

void MoviePanelAgent::UpdateData(const UpdateRequest& request)
{

}
